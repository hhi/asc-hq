/***************************************************************************
                          buildings.cpp  -  description
                             -------------------
    begin                : Sat Feb 17 2001
    copyright            : (C) 2001 by Martin Bickel
    email                : bickel@asc-hq.org
 ***************************************************************************/

/*! \file buildings.cpp
    \brief Implementation of the Building class 
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <algorithm>
#include <cmath>

#include "vehicletype.h"
#include "buildingtype.h"
#include "buildings.h"
#include "viewcalculation.h"
#include "errors.h"
#include "spfst.h"
#include "resourcenet.h"
#include "itemrepository.h"


const float repairEfficiencyBuilding[resourceTypeNum*resourceTypeNum] = { 1./3., 0,     1. / 3. ,
                                                                          0,     1./3., 0,
                                                                          0,     0,     0 };

Building :: Building ( pmap actmap, const MapCoordinate& _entryPosition, const pbuildingtype type, int player, bool setupImages )
           : ContainerBase ( type, actmap, player ), typ ( type ), repairEfficiency ( repairEfficiencyBuilding )
{
   int i;
   for ( i = 0; i < 8; i++ )
      aiparam[i] = NULL;

   _completion = 0;
   connection = 0;

   lastmineddist= 0;
   maxresearchpoints = 0;
   for ( i = 0; i < waffenanzahl; i++ )
      ammo[i] = 0;

   netcontrol = 0;

   for ( i = 0; i< 32; i++ ) {
      production[i] = 0;
      productionbuyable[i] = 0;
      loading[i] = 0;
   }

   repairedThisTurn = 0;
   researchpoints = 0;
   visible = 1;

   entryPosition = _entryPosition;

   gamemap->player[player].buildingList.push_back ( this );

   chainbuildingtofield ( entryPosition, setupImages );
   #ifdef karteneditor
   plus = maxplus = typ->maxplus;
   #endif
}


bool Building::canRepair ( void )
{
   if ( typ->special & cgrepairfacilityb )
      return true;
   else
      return false;
}


void Building :: convert ( int player )
{
   if (player > 8)
      fatalError("convertbuilding: \n color mu� im bereich 0..8 sein ");

   int oldcol = getOwner();

   #ifdef sgmain
   if ( oldcol == 8 )
      for ( int r = 0; r < 3; r++ )
         if ( gamemap->isResourceGlobal( r )) {
            gamemap->bi_resource[player].resource(r) += actstorage.resource(r);
            actstorage.resource(r) = 0;
         }

   #endif
   if ( color < 8*8 )
      removeview();

   gamemap->player[oldcol].queuedEvents++;
   gamemap->player[player].queuedEvents++;

   tmap::Player::BuildingList::iterator i = find ( gamemap->player[oldcol].buildingList.begin(), gamemap->player[oldcol].buildingList.end(), this );
   if ( i != gamemap->player[oldcol].buildingList.end())
      gamemap->player[oldcol].buildingList.erase ( i );

   gamemap->player[player].buildingList.push_back( this );

   color = player * 8;

   if ( player < 8 )
      addview();

   for ( int i = 0; i < 32; i++)
      if ( loading[i] )
         loading[i]->convert ( player );

   /*
   if ( connection & cconnection_conquer ) {
      gamemap->player[oldcol].queuedEvents++;
      gamemap->player[col].queuedEvents++;
      // releaseevent(NULL,this,cconnection_conquer);
   }

   if ( connection & cconnection_lose ) {
      gamemap->player[oldcol].queuedEvents++;
      // releaseevent(NULL,this,cconnection_lose);
      gamemap->player[col].queuedEvents++;
   } */
}



void* Building :: getpicture ( const BuildingType::LocalCoordinate& localCoordinate )
{
//                      if ( bld->typ->id == 8 ) {          // Windkraftwerk

   pfield fld = getField ( localCoordinate );
   if ( fld ) {
      int w = fld->getweather();

       if ( typ->w_picture[w][_completion][localCoordinate.x][localCoordinate.y] )
          return typ->w_picture[w][_completion][localCoordinate.x][localCoordinate.y];
       else
          return typ->w_picture[0][_completion][localCoordinate.x][localCoordinate.y];
   } else
      return NULL;
}


void Building::regroupUnits ()
{
   int num = 0;
   for ( int i = 18; i < 32; i++ )
      if ( loading[i] )
         num++;

   if ( num ) {
      for ( int i = 0; i < 18; i++ )
         if ( !loading[i] ) {
            for ( int j = i+1; j < 32; j++ )
               loading[j-1] = loading[j];
            loading[31] = NULL;
         }
   }
}


int Building :: vehicleloadable ( const pvehicle vehicle, int uheight ) const
{
   if ( uheight == -1 )
      uheight = vehicle->height;

   if ( vehicle->functions & cf_trooper )
      if ( uheight & (chschwimmend | chfahrend ))
         uheight |= (chschwimmend | chfahrend );  //these heights are effectively the same

   if ( getCompletion() ==  typ->construction_steps - 1  && typ->vehicleloadable(vehicle->typ ) )
      if ( typ->loadcapability & uheight ) {
         if ( (( typ->loadcapacity >= vehicle->size())               // the unit is physically able to get "through the door"
           && (( typ->unitheightreq & vehicle->typ->height ) || !typ->unitheightreq)
           && !( typ->unitheight_forbidden & vehicle->typ->height) )
                   ||
             ( vehicle->functions & cf_trooper )
           )
           if ( (vehiclesLoaded()+1 < maxloadableunits ) || vehicle->color != color ) {

         #ifdef karteneditor
              return 2;
         #else
              if ( color == vehicle->color )
                 return 2;
              else
                if ( !vehicle->attacked ) {
                   if ( color == (8 << 3) )      // neutral building can be conquered by any unit
                      return 2;
                   else
                      if ( (vehicle->functions & cf_conquer)  || ( damage >= mingebaeudeeroberungsbeschaedigung))
                         if ( getdiplomaticstatus2(color, vehicle->color ) == cawar )
                            return 2;
                }
         #endif
         }
      }

/*
&&
         (
         (( color == actmap->actplayer * 8)                              // ganz regul�r: eigenes geb�ude

         || (( vehicle->functions & cftrooper )                // JEDES Geb�ude mu� sich mit Fusstruppen erobern lassen
         && (( uheight == typ->height ) || (typ->height >= chschwimmend && hgt == chfahrend))
         && ( !vehicle->attacked ))
         // && color != (8 << 3)) )
         //&& ( typ->loadcapability & hgt ))
         ||

         ( (( damage >= mingebaeudeeroberungsbeschaedigung) || ( vehicle->functions & cfconquer ))    // bei Besch�digung oder cfconquer jedes Geb�ude mit fahrenden vehicle
         && (vehicle->height == chfahrend)
         // && ( color != (8 << 3))
         && ( !vehicle->attacked )
         && ( typ->loadcapability & hgt )
         && ( typ->height & vehicle->typ->height ))
         ||

         (( color == )                               // neutrale Geb�ude lassen sich immer erobern
         // && (vehicle->height == chfahrend)
         && ( !vehicle->attacked )
         && ( typ->loadcapability & hgt )
         && ( typ->height & vehicle->typ->height ) ))
       )

          return 2;
      else
           return 0;
*/
   return 0;
}


#ifndef sgmain
void Building :: execnetcontrol ( void ) {}
int Building :: putResource ( int amount, int resourcetype, int queryonly, int scope ) { return 0; };
int Building :: getResource ( int amount, int resourcetype, int queryonly, int scope ) { return 0; };
#endif

void Building :: setCompletion ( int completion, bool setupImages )
{
  _completion = completion;
  if ( setupImages )
     resetPicturePointers ( );
}





int  Building :: chainbuildingtofield ( const MapCoordinate& entryPos, bool setupImages )
{
   MapCoordinate oldpos = entryPosition;
   entryPosition = entryPos;

   for ( int a = 0; a < 4; a++)
      for ( int b = 0; b < 6; b++)
         if ( typ->getpicture ( BuildingType::LocalCoordinate( a, b) )) {
            pfield f = getField( BuildingType::LocalCoordinate( a, b) );
            if ( !f || f->building ) {
               entryPosition = oldpos;
               return 1;
            }
         }

   for ( int a = 0; a < 4; a++)
      for ( int b = 0; b < 6; b++)
         if ( typ->getpicture ( BuildingType::LocalCoordinate( a , b ) )) {
            pfield field = getField( BuildingType::LocalCoordinate( a, b) );

            tfield::ObjectContainer::iterator i = field->objects.begin();
            while ( i != field->objects.end()) {
               if ( !i->typ->canExistBeneathBuildings )
                  i = field->objects.erase ( i );
               else
                  i++;
            };

            if ( field->vehicle ) {
               delete field->vehicle;
               field->vehicle = NULL;
            }

            field->building = this;
           }

   for ( int i = 0; i < 32; i++ )
      if ( loading[i] )
         loading[i]->setnewposition ( entryPos.x, entryPos.y );

   pfield field = getField( typ->entry );
   if ( field )
      field->bdt |= getTerrainBitType(cbbuildingentry) ;

   if ( setupImages ) {
      resetPicturePointers ();
      gamemap->calculateAllObjects();
   }

   return 0;
}


int  Building :: unchainbuildingfromfield ( void )
{
   int set = 0;
   for (int i = 0; i <= 3; i++)
      for (int j = 0; j <= 5; j++)
         if ( typ->getpicture ( BuildingType::LocalCoordinate(i,j) ) ) {
            pfield fld = getField( BuildingType::LocalCoordinate(i,j) );
            if ( fld && fld->building == this ) {
               set = 1;
               fld->building = NULL;
               fld->picture = NULL;
               // if ( fld->vehicle )
               //   removevehicle( &fld->vehicle );

               TerrainBits t = getTerrainBitType(cbbuildingentry);
               t.flip();
               fld->bdt &= t;

               #ifdef sgmain
                if ( !gamemap->__mapDestruction )
                   if ( typ->destruction_objects[i][j] )
                      fld->addobject ( getobjecttype_forid ( typ->destruction_objects[i][j] ), -1, true );
               #endif

            }
         }

   return set;
}


void Building :: addview ( void )
{
   tcomputebuildingview bes ( gamemap );
   bes.init( this, +1 );
   bes.startsearch();
}

void Building :: removeview ( void )
{
   if ( color != 64 ) {
      tcomputebuildingview bes ( gamemap );
      bes.init( this, -1 );
      bes.startsearch();
   }
}


int Building :: getArmor ( void )
{
   return typ->_armor * gamemap->getgameparameter( cgp_buildingarmor ) / 100;
}

int Building :: gettank ( int resource )
{
   if ( gamemap && gamemap->_resourcemode == 1 )
      return typ->_bi_maxstorage.resource(resource);
   else
      return typ->_tank.resource(resource);
}



pfield        Building :: getField( const BuildingType::LocalCoordinate& lc ) const
{
  return gamemap->getField ( getFieldCoordinates ( lc ));
}


pfield        Building :: getEntryField( ) const
{
  return getField ( typ->entry );
}

MapCoordinate3D Building :: getEntry( ) const
{
  return MapCoordinate3D( entryPosition, typ->buildingheight);
}



MapCoordinate Building :: getFieldCoordinates ( const BuildingType::LocalCoordinate& lc ) const
{
  return typ->getFieldCoordinate ( entryPosition, lc );
}

void        Building :: resetPicturePointers ( void )
{
   if ( visible )
      for (int x = 0; x < 4; x++)
         for ( int y = 0; y < 6; y++ ) {
            BuildingType::LocalCoordinate lc ( x,y );
            if ( getpicture (lc) )
                getField ( lc )->picture = getpicture ( lc );
         }

}


void    Building :: produceAmmo ( int type, int num )
{
   num = ((num + weaponpackagesize - 1) / weaponpackagesize)*weaponpackagesize;
   Resources res;
   for( int j = 0; j< resourceTypeNum; j++ )
      res.resource(j) = cwaffenproduktionskosten[type][j] * num / 5;

   ContainerBase* cb = this;  // Really strange. Building is derived from Containerbase, but getResource doesn't work here
   Resources res2 = cb->getResource ( res, 1 );
   int perc = 100;
   for ( int i = 0; i< resourceTypeNum; i++ )
       if ( res.resource(i) )
          perc = min ( perc, 100 * res2.resource(i) / res.resource(i) );
   int produceable = num * perc / 100 ;
   int produceablePackages = produceable / 5;

   for( int k = 0; k< resourceTypeNum; k++ )
      res.resource(k) = cwaffenproduktionskosten[type][k] * produceablePackages;

   cb->getResource ( res, 0 );

   ammo[type] += produceablePackages * 5;
}


Building :: ~Building ()
{
   if ( gamemap ) {
      int c = color/8;

      tmap::Player::BuildingList::iterator i = find ( gamemap->player[c].buildingList.begin(), gamemap->player[c].buildingList.end(), this );
      if ( i != gamemap->player[c].buildingList.end() )
         gamemap->player[c].buildingList.erase ( i );

      for ( int j = 0; j < 8; j++ )
         gamemap->player[j].queuedEvents++;
   }

   for ( int i = 0; i < 32; i++ )
      if ( loading[i] )
         delete loading[i] ;

   unchainbuildingfromfield();

   /*
   int set = unchainbuildingfromfield();
   if ( set )
      for ( int i = xpos - 6; i <= xpos + 6; i++)
         for (j = ypos - 6; j <= ypos + 6; j++)
            if ((i >= 0) && (i < gamemap->xsize))
               if ((j >= 0) && (j < gamemap->ysize)) {
                  calculateobject(i,j,0   , streetobjectcontainer );
                  calculateobject(i,j,0   , railroadobject );
                  // calculateobject(i,j,true, powerlineobject );
                  // calculateobject(i,j,true, pipelineobject );
               }
   */

}


const int buildingstreamversion = -2;


void Building :: write ( tnstream& stream, bool includeLoadedUnits )
{
    stream.writeInt ( buildingstreamversion );

    stream.writeInt ( typ->id );
    int i;
    for ( i = 0; i< resourceTypeNum; i++ )
       stream.writeInt ( bi_resourceplus.resource(i) );
    stream.writeChar ( color );
    stream.writeWord ( getEntry().x );
    stream.writeWord ( getEntry().y );
    stream.writeChar ( getCompletion() );
    for ( i = 0; i < waffenanzahl; i++ )
       stream.writeWord ( 0 ); // was: ammoautoproduction

    for ( i = 0; i< resourceTypeNum; i++ )
       stream.writeInt ( plus.resource(i) );

    for ( i = 0; i< resourceTypeNum; i++ )
       stream.writeInt ( maxplus.resource(i) );

    for ( i = 0; i< resourceTypeNum; i++ )
       stream.writeInt ( actstorage.resource(i) );

    for ( i = 0; i< waffenanzahl; i++ )
       stream.writeWord ( ammo[i] );

    stream.writeWord ( maxresearchpoints );
    stream.writeWord ( researchpoints );
    stream.writeChar ( visible );
    stream.writeChar ( damage );
    stream.writeInt  ( netcontrol );
    stream.writeString ( name );

    stream.writeInt ( repairedThisTurn );

    char c = 0;

    if ( includeLoadedUnits )
       if (typ->loadcapacity )
          for ( int k = 0; k <= 31; k++)
             if (loading[k] )
                c++;

    stream.writeChar ( c );
    if ( c )
       for ( int k = 0; k <= 31; k++)
          if ( loading[k] )
             loading[k]->write ( stream );


    c = 0;
    if (typ->special & cgvehicleproductionb )
       for (int k = 0; k <= 31; k++)
          if ( production[k] )
             c++;

    stream.writeChar ( c );
    if ( c )
       for (int k = 0; k <= 31; k++)
          if (production[k] )
             stream.writeWord( production[k]->id );


    c = 0;
    if (typ->special & cgvehicleproductionb )
       for (int k = 0; k <= 31; k++)
          if ( productionbuyable[k] )
             c++;

    stream.writeChar ( c );
    if ( c )
       for ( int k = 0; k <= 31; k++)
          if ( productionbuyable[k] )
             stream.writeWord( productionbuyable[k]->id );

}


Building* Building::newFromStream ( pmap gamemap, tnstream& stream )
{
    int version = stream.readInt();
    int xpos, ypos, color;
    Resources res;

    pbuildingtype typ;


    if ( version == buildingstreamversion || version == -1 ) {

       int id = stream.readInt ();
       typ = gamemap->getbuildingtype_byid ( id );
       if ( !typ )
          throw InvalidID ( "building", id );

       for ( int i = 0; i < 3; i++ )
          res.resource(i) = stream.readInt();

       color = stream.readChar();
       xpos = stream.readWord() ;
    } else {
       int id = version;

       typ = gamemap->getbuildingtype_byid ( id );
       if ( !typ )
          throw InvalidID ( "building", id );

       color = stream.readChar();
       xpos  = stream.readWord();
    }

    ypos = stream.readWord();

    Building* bld = new Building ( gamemap, MapCoordinate(xpos,ypos), typ, color/8, false );
    bld->bi_resourceplus = res;
    bld->readData ( stream, version );
    return bld;
}



void Building:: read ( tnstream& stream )
{
    int version = stream.readInt();

    if ( version == buildingstreamversion || version == -1 ) {
       stream.readInt (); // id
       for ( int i = 0; i < 3; i++ )
          bi_resourceplus.resource(i) = stream.readInt();

       stream.readChar(); // color
       stream.readWord() ; // xpos
    } else {
       // int id = version;
       stream.readChar(); // color
       stream.readWord(); // xpos
       bi_resourceplus = Resources ( 0, 0, 0);
    }

    stream.readWord(); // ypos
    readData ( stream, version );
}


void Building :: readData ( tnstream& stream, int version )
{
    setCompletion ( stream.readChar(), false );

    int i;
    for ( i = 0; i < waffenanzahl; i++)
       stream.readWord(); // was : ammoautoproduction

    for ( i = 0; i< 3; i++ )
       plus.resource(i) = stream.readInt();

    for ( i = 0; i< 3; i++ )
       maxplus.resource(i) = stream.readInt();

    for ( i = 0; i< 3; i++ )
       actstorage.resource(i) = stream.readInt();

    for ( i = 0; i < waffenanzahl; i++)
       ammo[i] = stream.readWord();

    maxresearchpoints = stream.readWord();
    researchpoints = stream.readWord();

    visible = stream.readChar();
    damage = stream.readChar();
    netcontrol = stream.readInt();
    name = stream.readString ();

    if ( version == -2 )
       repairedThisTurn = stream.readInt ( );
    else
       repairedThisTurn = 0;

    char c = stream.readChar();
    if ( c ) {
       for ( int k = 0; k < c; k++) {
          loading[k] = Vehicle::newFromStream ( gamemap, stream );
          loading[k]->setnewposition ( getEntry().x, getEntry().y );
          if ( loading[k]->color != color ) {
             ASCString msg;
             msg.format("warning: the building at position %d / %d , which is owned by player %d, contained units from player %d ", getEntry().x, getEntry().y, color/8, loading[k]->color/8 );
             warning(msg);
             loading[k]->convert(getOwner());
          }
       }
       for ( int l = c; l < 32; l++ )
          loading[l] = NULL;
    }

    c = stream.readChar();
    if ( c ) {
       for ( int k = 0; k < c ; k++) {
           word id = stream.readWord();
           production[k] = gamemap->getvehicletype_byid ( id ) ;
           if ( !production[k] )
              throw InvalidID ( "unit", id );
       }
       for ( int l = c; l < 32; l++ )
          production[l] = NULL;
    }

    c = stream.readChar();
    if ( c ) {
       for ( int k = 0; k < c ; k++) {
           word id = stream.readWord();
           productionbuyable[k] = gamemap->getvehicletype_byid ( id );

           if ( !productionbuyable[k] )
              throw InvalidID ( "unit", id );
       }
       for ( int l = c; l < 32; l++ )
          productionbuyable[l] = NULL;
    }
}

const ASCString& Building::getName ( ) const
{
   if ( name.empty())
      return typ->name;
   else
      return name;
}

Resources Building::netResourcePlus( ) const
{
   Resources res;
   for ( int resourcetype = 0; resourcetype < resourceTypeNum; resourcetype++ ) {
      GetResourcePlus grp;
      res.resource(resourcetype) += grp.getresource ( getEntry().x, getEntry().y, resourcetype, color/8, 1 );
   }
   return res;
}



Building::Work* Building::spawnWorkClasses( bool justQuery )
{
   if ( actmap->_resourcemode != 1 ) {
      if ( typ->special & cgwindkraftwerkb )
         return new WindPowerplant ( this );

      if ( typ->special & cgsolarkraftwerkb )
         return new SolarPowerplant ( this );

      if ( typ->special & cgconventionelpowerplantb )
         return new MatterConverter ( this );

      if ( typ->special & cgminingstationb )
         return new MiningStation ( this, justQuery );
   } else {
      return new BiResourceGeneration ( this );
   }

   return NULL;
}




float  getminingstationeficency ( int dist )
{
  // f ( x ) = a / ( b * ( x + d ) ) - c

double a,b,c,d;

a          =     10426.400 ;
b          =     1.0710969 ;
c          =     568.88887 ;
d          =     6.1111109 ;

   return (a / ( b * (dist + d)) - c) / 1024;
}


GetMiningInfo::MiningInfo::MiningInfo()
{
   for ( int i = 0; i < maxminingrange+2; i++ )
      efficiency[i] = 0;
}

GetMiningInfo :: GetMiningInfo ( pmap _gamemap ) : SearchFields ( _gamemap )
{
}

void GetMiningInfo :: testfield ( const MapCoordinate& mc )
{
   pfield fld = gamemap->getField ( mc );
   if ( miningInfo.efficiency[ dist ] == 0 )
      miningInfo.efficiency[ dist ] = int(getminingstationeficency ( dist ) * 1024);

   miningInfo.avail[dist].material += fld->material * resource_material_factor;
   miningInfo.avail[dist].fuel     += fld->fuel     * resource_fuel_factor;
   miningInfo.max[dist].material   += 255 * resource_material_factor;
   miningInfo.max[dist].fuel       += 255 * resource_fuel_factor;
}


void GetMiningInfo :: run (  const pbuilding bld )
{
   initsearch ( bld->getEntry(), maxminingrange, 0 );
   startsearch();
}




Building::MatterConverter :: MatterConverter( Building* _bld ) : bld ( _bld ), percentage ( 100 )
{
}

bool Building::MatterConverter :: run()
{
   int perc = percentage;

   int usageNum = 0;
   for ( int r = 0; r < 3; r++ )
      if ( bld->plus.resource(r) < 0 )
         ++usageNum;

   if ( usageNum > 0 ) {
      // if the resource generation requires other resources, don't waste anything
      // by producing more than storage capacity available

      for ( int r = 0; r < 3; r++ )
         if ( bld->plus.resource(r) > 0 ) {
            int p = bld->putResource ( bld->plus.resource(r), 0, 1 );

            if ( perc > 100 * p / bld->plus.resource(r) )
               perc = 100 * p / bld->plus.resource(r) ;
         }
   }

   Resources toGet = bld->plus * perc / 100  ;
   for ( int r = 0; r < 3; r++ )
      if ( toGet.resource(r) < 0 )
         toGet.resource(r)  = - toGet.resource(r) ;
      else
         toGet.resource(r) = 0;


   Resources avail = bld->getResource ( toGet, 1 );

   for ( int r = 0; r < 3; r++ ) {
      if ( bld->plus.resource(r) < 0 ) {
         int p = 100 * avail.resource(r) / -bld->plus.resource(r);
         if ( p < perc )
            perc = p;
      }
   }


   bool didSomething = false;

   for ( int r = 0; r < 3; r++ )
      if ( bld->plus.resource(r) > 0 ) {
         bld->putResource( bld->plus.resource(r) * perc  / 100, r , 0);
         if ( bld->plus.resource(r) * perc / 100  > 0)
            didSomething = true;

      } else {
         if ( bld->plus.resource(r) < 0 )
            bld->getResource( -bld->plus.resource(r) * perc  / 100, r , 0);
      }

   percentage -= perc;
   return didSomething;
}


bool Building::MatterConverter :: finished()
{
   return percentage == 0;
}

Resources Building::MatterConverter :: getPlus()
{
  Resources r;
  for ( int i = 0; i < 3; i++ )
     if ( bld->plus.resource(i) > 0 )
        r.resource(i) = bld->plus.resource(i);
  return r;
}

Resources Building::MatterConverter :: getUsage()
{
  Resources r;
  for ( int i = 0; i < 3; i++ )
     if ( bld->plus.resource(i) < 0 )
        r.resource(i) = -bld->plus.resource(i);
  return r;
}

/*
Research :: Research( Building* _bld ) : bld ( _bld ), percentage ( 100 )
{
}

bool Research :: run()
{
   int perc = percentage;

   int usageNum = 0;
   for ( int r = 0; r < 3; r++ )
      if ( plus.resource(r) < 0 )
         ++usageNum;

   if ( usageNum > 0 ) {
      // if the resource generation requires other resources, don't waste anything
      // by producing more than storage capacity available

      for ( int r = 0; r < 3; r++ )
         if ( bld->plus.resource(r) > 0 ) {
            int p = putResource ( bld->plus.resource(r), 0, 1 );

            if ( perc > 100 * p / bld->plus.resource(r) )
               perc = 100 * p / bld->plus.resource(r) ;
         }
   }

   Resources toGet = bld->plus * perc / 100  ;
   for ( int r = 0; r < 3; r++ )
      if ( toGet.resource(r) < 0 )
         toGet.resource(r)  = - toGet.resource(r) ;
      else
         toGet.resource(r) = 0;


   Resources avail = bld->getResource ( toGet, 1 );

   for ( int r = 0; r < 3; r++ ) {
      if ( bld->plus.resource(r) ) {
         int p = 100 * avail.resource(r) / bld->plus.resource(r);
         if ( p < perc )
            perc = P;
      }
   }


   bool didSomething = false;

   for ( int r = 0; r < 3; r++ )
      if ( bld->plus.resource(r) > 0 ) {
         bld->putResource( bld->plus.resource(r) * perc  / 100, r , 0);
         if ( bld->plus.resource(r) * perc / 100  > 0)
            didSomething = true;

      } else {
         bld->getResource( -bld->plus.resource(r) * perc  / 100, r , 0);
      }

   percentage = perc;
   return didSomething;
}


bool Research :: finished()
{
   return percentage == 0;
}

Resources Research :: getUsage()
{
  Resources r;
  for ( int i = 0; i < 3; i++ 9
     if ( bld->plus.resource(r) < 0 )
        r.resource(r) = -bld->plus.resource(r)
  return r;
}
   **/

Building::RegenerativePowerPlant :: RegenerativePowerPlant( Building* _bld ) : bld ( _bld )
{
}

bool Building::RegenerativePowerPlant :: finished()
{
   for( int r = 0; r < 3; r++ )
      if ( toProduce.resource(r) > 0 )
         return false;
   return true;
}

bool Building::RegenerativePowerPlant :: run()
{
   Resources tp = bld->putResource( toProduce , 0 );
   bool didSomething = false;
   for  ( int r = 0; r < 3; r++ )
      if ( tp.resource(r) ) {
         didSomething = true;
         toProduce.resource(r) -= tp.resource(r);
      }
   return didSomething;
}

Resources Building::RegenerativePowerPlant :: getUsage()
{
   return Resources();
}

Resources Building::WindPowerplant :: getPlus()
{
   Resources p;
   for ( int r = 0; r < 3; r++ )
      p.resource(r) =  bld->maxplus.resource(r) * actmap->weather.wind[0].speed / 255;
   return p;
}

Resources Building::SolarPowerplant :: getPlus()
{
   int sum = 0;
   int num = 0;
   for ( int x = 0; x < 4; x++ )
      for ( int y = 0; y < 6; y++)
         if ( bld->getpicture ( BuildingType::LocalCoordinate(x, y) ) ) {
            pfield fld = bld->getField ( BuildingType::LocalCoordinate(x, y) );
            int weather = 0;
            while ( fld->typ != fld->typ->terraintype->weather[weather] )
               weather++;

            sum += csolarkraftwerkleistung[weather];
            num ++;
         }

   Resources rplus;
   for ( int r = 0; r < 3; r++ )
      rplus.resource(r) = bld->maxplus.resource(r) * sum / ( num * 1024 );
   return rplus;
}


Resources Building::BiResourceGeneration::getPlus()
{
   return bld->bi_resourceplus;
}




Building::MiningStation :: MiningStation( Building* bld_  , bool justQuery_) : bld ( bld_ ), justQuery( justQuery_ ), SearchFields ( bld_->getMap() )
{
   for ( int r = 1; r < 3; r++ )
      if ( bld->plus.resource(r) > 0 )
         toExtract_thisTurn.resource(r) = bld->plus.resource(r);

   if( justQuery ) {
      hasRun = false;
      run();
      hasRun = true;
   } else
      hasRun = false;
}

bool Building::MiningStation :: run()
{
   if ( justQuery && hasRun )
      return false;

   extracted = Resources();
   int perc = 100;
   if ( !justQuery ) {
      Resources netAvail = bld->putResource( toExtract_thisTurn, 1 );
      for ( int r = 0 ; r < 3 ; r++ )
         if ( toExtract_thisTurn.resource(r) > 0 )
            perc = min ( perc, 100 * netAvail.resource(r) / toExtract_thisTurn.resource(r) );
   }
   toExtract_thisLoop = toExtract_thisTurn * perc / 100;

   perc = 100;
   if ( !justQuery ) {
      Resources storable = bld->putResource( toExtract_thisLoop, 1 );
      for ( int r = 0; r < 3; r++ )
         if ( toExtract_thisLoop.resource(r) )
            perc = min ( perc, 100 * storable.resource(r) / toExtract_thisLoop.resource(r) );
   }

   // check how much resources the production of toExtract_thisTurn would need
   Resources toConsume;
   int absperc;
   for ( int r = 0; r < 3; r++ )
      if ( bld->plus.resource(r) > 0 )
         absperc = 1000 * toExtract_thisTurn.resource(r) / bld->plus.resource(r);

   for ( int r = 0; r < 3; r++ )
      if ( bld->plus.resource(r) < 0 )
         toConsume.resource(r) = -bld->plus.resource(r) * absperc / 1000;

   if ( !justQuery ) {
   // how much of it is available ?
      Resources avail = bld->getResource( toConsume, 1 );

      for ( int r = 0; r < 3; r++ )
         if ( toConsume.resource(r) )
            perc = min ( perc, 100 * avail.resource(r) / toConsume.resource(r) );
   }

   // now all limitations have been considered...
   for ( int r = 0; r < 3; r++ )
      toExtract_thisLoop.resource(r) = toExtract_thisLoop.resource(r) * perc / 100;


   initsearch( bld->getEntry(), 0, maxminingrange );
   startsearch();

   perc = 100;
   for ( int r = 0 ; r < 3 ; r++ )
      if ( bld->plus.resource(r) > 0 ) {
         int p = 100 * extracted.resource(r) / bld->plus.resource(r);
         if ( p < perc )
            perc = p;
      }

   consumed = bld->plus * perc / -100;
   for ( int r = 0; r < 3; r++ )
     if ( consumed.resource(r) < 0 )
        consumed.resource(r) = 0;

   if ( !justQuery) {
      bld->getResource(consumed, 0 );
      bld->putResource(extracted, 0 );
   }

   toExtract_thisTurn -= extracted;

   for ( int r = 0; r < 3; r++ )
      if ( extracted.resource(r) > 0 )
         return true;

   return false;
}

void Building::MiningStation :: testfield ( const MapCoordinate& mc )
{
   cancelSearch = true;
   for ( int r = 0; r < 3; r++ )
      if ( toExtract_thisLoop.resource(r) > 0 )
         cancelSearch = false;

   if ( cancelSearch == false ) {
      pfield fld = gamemap->getField ( mc );
      float distefficiency = getminingstationeficency ( dist );

      for ( int r = 1; r < 3; r++ ) {
         if ( toExtract_thisLoop.resource(r) > 0 ) {
            float e = toExtract_thisLoop.resource(r) * distefficiency;
            float got = 0;
            float buildingEfficiency;
            if ( r == 1)
               buildingEfficiency =  double(bld->typ->efficiencymaterial) / 1024;
            else
               buildingEfficiency =  double(bld->typ->efficiencyfuel) / 1024;

            if ( resource_material_factor * buildingEfficiency == 0)
               return;

            if ( r == 1 ) {
               got = min( fld->material * resource_material_factor  * buildingEfficiency, e );
               if ( !justQuery )
                  fld->material -= int( ceil( got / (resource_material_factor  * buildingEfficiency)));
            } else {
               got = min( fld->fuel * resource_fuel_factor  * buildingEfficiency, e );
               if ( !justQuery )
                  fld->fuel -= int( ceil( got  / (resource_fuel_factor * buildingEfficiency)));
            }
            toExtract_thisLoop.resource(r) -= int(ceil(got / distefficiency));
            if ( toExtract_thisLoop.resource(r) < 0 )
               toExtract_thisLoop.resource(r) = 0;

            extracted.resource(r) += int(got);

            if ( !justQuery ) {
               if ( !fld->resourceview )
                  fld->resourceview = new tfield::Resourceview;
               fld->resourceview->visible |= 1 << bld->getOwner();
               fld->resourceview->fuelvisible[bld->getOwner()] = fld->fuel;
               fld->resourceview->materialvisible[bld->getOwner()] = fld->material;
            }
         }
      }
   }
}

bool Building::MiningStation :: finished()
{
   for ( int r = 0; r < 3; r++ )
      if ( toExtract_thisTurn.resource(r) )
         return false;
   return true;
}

Resources Building::MiningStation :: getPlus()
{
   return extracted;
}

Resources Building::MiningStation :: getUsage()
{
   return consumed;
}




Resources Building :: getResourcePlus( )
{
  Work* w = spawnWorkClasses ( true );
  Resources r;
  if ( w )
    r = w->getPlus();
  delete w;
  return r;
}

Resources Building :: getResourceUsage( )
{
  Work* w = spawnWorkClasses ( true );
  Resources r;
  if ( w )
    r = w->getUsage();
  delete w;
  return r;
}


/*
void Building :: getresourceusage ( Resources* usage )
{
   returnresourcenuseforpowerplant ( this, 100, usage, 0 );
   if ( typ->special & cgresearchb ) {
      int material;
      int energy;
      returnresourcenuseforresearch ( this, researchpoints, &energy, &material );
      usage->material += material;
      usage->energy   += energy;
      usage->fuel  = 0;
   }
}
*/

