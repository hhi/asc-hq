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

#include "vehicletype.h"
#include "buildingtype.h"
#include "buildings.h"
#include "viewcalculation.h"
#include "errors.h"
#include "spfst.h"


const float repairEfficiencyBuilding[resourceTypeNum*resourceTypeNum] = { 1./3., 0,     1. / 3. ,
                                                                          0,     1./3., 0,
                                                                          0,     0,     0 };

Building :: Building ( pmap actmap, const MapCoordinate& _entryPosition, const pbuildingtype type, int player, bool setupImages )
           : ContainerBase ( type, actmap, player ), repairEfficiency ( repairEfficiencyBuilding ), typ ( type )
{
   int i;
   for ( i = 0; i < 8; i++ )
      aiparam[i] = NULL;

   _completion = 0;
   connection = 0;

   lastenergyavail = 0;
   lastmaterialavail = 0;
   lastfuelavail = 0;
   lastmineddist= 0;
   maxresearchpoints = 0;
   for ( i = 0; i < waffenanzahl; i++ ) {
      munition[i] = 0;
      munitionsautoproduction[i] = 0;
   }
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
}


bool Building::canRepair ( void )
{
   return typ->special & cgrepairfacilityn;
}


void Building :: convert ( int col )
{
   if (col > 8)
      fatalError("convertbuilding: \n color mu� im bereich 0..8 sein ");

   int oldcol = color >> 3;

   #ifdef sgmain
   if ( oldcol == 8 )
      for ( int r = 0; r < 3; r++ )
         if ( gamemap->isResourceGlobal( r )) {
            gamemap->bi_resource[col].resource(r) += actstorage.resource(r);
            actstorage.resource(r) = 0;
         }

   #endif

   gamemap->player[oldcol].queuedEvents++;

   tmap::Player::BuildingList::iterator i = find ( gamemap->player[oldcol].buildingList.begin(), gamemap->player[oldcol].buildingList.end(), this );
   if ( i != gamemap->player[oldcol].buildingList.end())
      gamemap->player[oldcol].buildingList.erase ( i );

   gamemap->player[col].buildingList.push_back( this );

   color = col << 3;

   for ( int i = 0; i < 32; i++)
      if ( loading[i] )
         loading[i]->convert ( col );

   if ( connection & cconnection_conquer )
      gamemap->player[oldcol].queuedEvents++;
      // releaseevent(NULL,this,cconnection_conquer);

   if ( connection & cconnection_lose )
      gamemap->player[oldcol].queuedEvents++;
      // releaseevent(NULL,this,cconnection_lose);
}



void* Building :: getpicture ( const BuildingType::LocalCoordinate& localCoordinate )
{
//                      if ( bld->typ->id == 8 ) {          // Windkraftwerk

   pfield fld = getField ( localCoordinate );
   if ( fld ) {
      int w = fld->getweather();

      #ifdef HEXAGON
       if ( typ->w_picture[w][_completion][localCoordinate.x][localCoordinate.y] )
          return typ->w_picture[w][_completion][localCoordinate.x][localCoordinate.y];
       else
          return typ->w_picture[0][_completion][localCoordinate.x][localCoordinate.y];
      #else
       return typ->picture[_completion][localCoordinate.x][localCoordinate.y];
      #endif
   } else
      return NULL;
}


int Building :: vehicleloadable ( pvehicle vehicle, int uheight ) const
{
   if ( uheight == -1 )
      uheight = vehicle->height;

   if ( vehicle->functions & cf_trooper )
      if ( uheight & (chschwimmend | chfahrend ))
         uheight |= (chschwimmend | chfahrend );  //these heights are effectively the same

   if ( getCompletion() ==  typ->construction_steps - 1 )
      if ( typ->loadcapability & uheight ) {
         if ( (( typ->loadcapacity >= vehicle->size())               // the unit is physically able to get "through the door"
           && ( vehiclesLoaded()+1 < maxloadableunits )
           && (( typ->unitheightreq & vehicle->typ->height ) || !typ->unitheightreq)
           && !( typ->unitheight_forbidden & vehicle->typ->height) )
                   ||
             ( vehicle->functions & cf_trooper )
           ) {
         //  && ( (uheight == typ->buildingheight)  || (typ->buildingheight >= chschwimmend && hgt == chfahrend) ))) {

         #ifdef karteneditor
              return 2;
         #else
              if ( color == gamemap->actplayer * 8)
                 return 2;
              else
                if ( !vehicle->attacked ) {
                   if ( color == (8 << 3) )      // neutral building can be conquered by any unit
                      return 2;
                   else
                      if ( (vehicle->functions & cf_conquer)  || ( damage >= mingebaeudeeroberungsbeschaedigung))
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
int  Building :: getresourceplus ( int mode, Resources* plus, int queryonly ) { return 0;};
void Building :: execnetcontrol ( void ) {}
int  Building :: processmining ( int res, int abbuchen ) { return 0; }
void Building :: getresourceusage ( Resources* usage ) {  usage->energy = 0;
                                                           usage->material =  0;
                                                           usage->fuel = 0;
                                                         }
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
            field->objects.clear();

            if ( field->vehicle ) {
               delete field->vehicle;
               field->vehicle = NULL;
            }

            field->building = this;

            // field->picture = gbde->typ->picture[compl][a - orgx][b - orgy];
            field->bdt &= ~( cbstreet | cbrailroad | cbpipeline | cbpowerline );
           }

   for ( int i = 0; i < 32; i++ )
      if ( loading[i] )
         loading[i]->setnewposition ( entryPos.x, entryPos.y );

   pfield field = getField( typ->entry );
   if ( field )
      field->bdt |= cbbuildingentry ;

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

               tterrainbits t1 = cbstreet | cbbuildingentry | cbrailroad | cbpowerline | cbpipeline;
               tterrainbits t2 = ~t1;

               fld->bdt &= t2;

               #ifdef sgmain
                if ( typ->destruction_objects[i][j] )
                   fld->addobject ( getobjecttype_forid ( typ->destruction_objects[i][j] ), -1, 1 );

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
   tcomputebuildingview bes ( gamemap );
   bes.init( this, -1 );
   bes.startsearch();
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



pfield        Building :: getField( const BuildingType::LocalCoordinate& lc )
{
  return gamemap->getField ( getFieldCoordinates ( lc ));
}


pfield        Building :: getEntryField( )
{
  return getField ( typ->entry );
}

MapCoordinate Building :: getEntry( )
{
  return entryPosition;
}



MapCoordinate Building :: getFieldCoordinates ( const BuildingType::LocalCoordinate& lc )
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
   num = ((num +4) / 5)*5;
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

   munition[type] += produceablePackages * 5;
}

void Building :: getpowerplantefficiency ( int* material, int* fuel )
{
   *material = typ->efficiencymaterial;
   *fuel = typ->efficiencyfuel;
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

   int set = unchainbuildingfromfield();

   /*
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
       stream.writeWord ( munitionsautoproduction[i] );

    for ( i = 0; i< resourceTypeNum; i++ )
       stream.writeInt ( plus.resource(i) );

    for ( i = 0; i< resourceTypeNum; i++ )
       stream.writeInt ( maxplus.resource(i) );

    for ( i = 0; i< resourceTypeNum; i++ )
       stream.writeInt ( actstorage.resource(i) );

    for ( i = 0; i< waffenanzahl; i++ )
       stream.writeWord ( munition[i] );

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
       int id = stream.readInt ();
       for ( int i = 0; i < 3; i++ )
          bi_resourceplus.resource(i) = stream.readInt();

       int color = stream.readChar();
       int xpos = stream.readWord() ;
    } else {
       int id = version;
       int color = stream.readChar();
       int xpos  = stream.readWord();
       bi_resourceplus = Resources ( 0, 0, 0);
    }

    int ypos = stream.readWord();
    readData ( stream, version );
}


void Building :: readData ( tnstream& stream, int version )
{
    setCompletion ( stream.readChar(), false );

    int i;
    for ( i = 0; i < waffenanzahl; i++)
       munitionsautoproduction[i] = stream.readWord();

    for ( i = 0; i< 3; i++ )
       plus.resource(i) = stream.readInt();

    for ( i = 0; i< 3; i++ )
       maxplus.resource(i) = stream.readInt();

    for ( i = 0; i< 3; i++ )
       actstorage.resource(i) = stream.readInt();

    for ( i = 0; i < waffenanzahl; i++)
       munition[i] = stream.readWord();

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
