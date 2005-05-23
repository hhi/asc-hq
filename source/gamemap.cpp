/*! \file gamemap.cpp
    \brief Implementation of THE central asc class: tmap 
*/

/***************************************************************************
                          gamemap.cpp  -  description
                             -------------------
    begin                : Tue Oct 24 2000
    copyright            : (C) 2000 by Martin Bickel
    email                : bickel@asc-hq.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <algorithm>
#include <ctime>
#include <cmath>

#include "global.h"
#include "misc.h"
#include "typen.h"
#include "vehicletype.h"
#include "buildingtype.h"
#include "spfst.h"
#include "dlg_box.h"
#include "dialog.h"
#include "itemrepository.h"
#include "strtmesg.h"
#include "graphics/blitter.h"
#include "overviewmapimage.h"
#include "iconrepository.h"

#ifdef sgmain
 #include "network.h"
 #include "gameoptions.h"
 #include "resourcenet.h"
#endif


RandomGenerator::RandomGenerator(int seedValue){

}

RandomGenerator::~RandomGenerator(){


}

unsigned int RandomGenerator::getPercentage(){
  return getRandomValue(100);
}

unsigned int RandomGenerator::getRandomValue(int limit) {
  return getRandomValue(0, limit); 
}

unsigned int RandomGenerator::getRandomValue (int lowerLimit, int upperLimit){
   if(upperLimit == 0) {
        return 1;
   }
   int random_integer = rand();
   random_integer = random_integer % upperLimit;
   return (lowerLimit + random_integer);
}




OverviewMapHolder :: OverviewMapHolder( tmap& gamemap ) : map(gamemap), initialized(false), completed(false), x(0),y(0) 
{
   idleEvent.connect( SigC::slot( *this, &OverviewMapHolder::idleHandler ));
}

bool OverviewMapHolder :: idleHandler( )
{
   int t = ticker;
   while ( !completed && (t + 5 > ticker ))
      drawNextField( true );
   return true;
}


void OverviewMapHolder::updateField( const MapCoordinate& pos )
{
   SPoint imgpos = OverviewMapImage::map2surface( pos );

   pfield fld = map.getField( pos );
   VisibilityStates visi = fieldVisibility( fld, map.playerView );
   if ( visi == visible_not ) {
      static SDLmm::Color invisible = 0;
      if ( !invisible ) {
         Surface& hex = IconRepository::getIcon("hexinvis.raw");
         invisible = overviewMapImage.GetPixelFormat().MapRGB ( hex.GetPixelFormat().GetRGB( hex.GetPixel ( hex.w() / 2, hex.h() / 2 )));
      }
      OverviewMapImage::fill ( overviewMapImage, imgpos, invisible );
   } else {
      if ( fld->building && fieldvisiblenow( fld, map.playerView) )
         OverviewMapImage::fill ( overviewMapImage, imgpos, map.player[fld->building->getOwner()].getColor() );
      else {

         int w = fld->getweather();
         fld->typ->getQuickView()->blit( overviewMapImage, imgpos );
         for ( tfield::ObjectContainer::iterator i = fld->objects.begin(); i != fld->objects.end(); ++i )
            if ( visi > visible_ago || i->typ->visibleago )
               i->getOverviewMapImage( w )->blit( overviewMapImage, imgpos );

         if ( fld->vehicle && fieldvisiblenow( fld, map.playerView) )
            OverviewMapImage::fillCenter ( overviewMapImage, imgpos, map.player[fld->vehicle->getOwner()].getColor() );

         if ( visi == visible_ago )
            OverviewMapImage::lighten( overviewMapImage, imgpos, 0.7 ); 

      }

   }
}

void OverviewMapHolder::drawNextField( bool signalOnComplete )
{
   if ( !init() )
      return;
      
   if ( x == map.xsize ) {
      x = 0;
      ++y;
   }   
   if ( y < map.ysize ) {
      updateField( MapCoordinate(x,y));
      ++x;
   }
   if ( y == map.ysize ) {
      completed = true;
      if ( signalOnComplete )
         viewChanged();
   }
}

bool OverviewMapHolder::init()
{
   if ( !initialized ) {
      if ( map.xsize > 0 && map.ysize > 0 ) {
         overviewMapImage = Surface::createSurface( (map.xsize+1) * 6, 4 + map.ysize * 2 , 32, 0 );
         initialized = true;
      }
   }
   return initialized;
}   


const Surface& OverviewMapHolder::getOverviewMap( bool complete )
{
   bool initialized = init();
   assert( initialized );
   if ( complete )
      while ( !completed )
         drawNextField( false );
   return overviewMapImage;
}

void OverviewMapHolder::startUpdate()
{
   completed = false;
   x = 0;
   y = 0;
}

void OverviewMapHolder::clear()
{
   overviewMapImage.Fill( Surface::transparent );
   startUpdate();
}



tmap :: tmap ( void )
      : overviewMapHolder( *this )
{
   randomSeed = rand();

   eventID = 0;

   __mapDestruction = false;
   int i;

   xsize = 0;
   ysize = 0;
   xpos = 0;
   ypos = 0;
   field = NULL;
   codeword[0] = 0;
   campaign = NULL;

   actplayer = -1;
   time.abstime = 0;

   _resourcemode = 0;

   for ( i = 0; i < 8; i++ )
      for ( int j = 0; j < 8; j++ )
          alliances[i][j] = cawar;

   for ( i = 0; i< 9; i++ ) {
      player[i].ai = NULL;
      player[i].player = i;

      if ( i == 0 )
         player[i].stat = Player::human;
      else
         player[i].stat = Player::off;

      player[i].queuedEvents = 0;
      if ( i < 8 ) {
         player[i].humanname = "human ";
         player[i].humanname += strrr( i );
         player[i].computername = "computer ";
         player[i].computername += strrr( i );
      } else
         player[i].humanname = player[i].computername = "neutral";


      player[i].research.chainToMap ( this, i );
      player[i].ASCversion = 0;
   }

   unitnetworkid = 0;

   levelfinished = 0;

   network = 0;

   for ( i = 0; i < 8; i++ ) {
      cursorpos.position[i].cx = 0;
      cursorpos.position[i].sx = 0;
      cursorpos.position[i].cy = 0;
      cursorpos.position[i].sy = 0;
   }

   messageid = 0;
   for ( i = 0; i< 8; i++ )
      alliances_at_beginofturn[i] = 0;

   shareview = NULL;
   continueplaying = 0;
   replayinfo = NULL;
   playerView = -1;
   lastjournalchange.abstime = 0;
   ellipse = 0;
   graphicset = 0;
   gameparameter_num = 0;
   game_parameter = NULL;
   mineralResourcesDisplayed = 0;

   weatherSystem  = new WeatherSystem(this, 1, 0.03);   
   setgameparameter( cgp_objectsDestroyedByTerrain, 1 );
}


const int tmapversion = 9;

void tmap :: read ( tnstream& stream )
{
   int version;
   int i;

   xsize = stream.readWord();
   ysize = stream.readWord();

   if ( xsize == 0xfffe  && ysize == 0xfffc ) {
     version = stream.readInt();
     if ( version > tmapversion )
        throw tinvalidversion ( "tmap", tmapversion, version );

     xsize = stream.readInt();
     ysize = stream.readInt();
   } else
      version = 1;

   xpos = stream.readWord();
   ypos = stream.readWord();
   stream.readInt(); // dummy
   field = NULL;
   stream.readdata ( codeword, 11 ); // endian ok !

   if ( version < 2 )
      ___loadtitle = stream.readInt();
   else
      ___loadtitle = true;

   bool loadCampaign = stream.readInt();
   actplayer = stream.readChar();
   time.abstime = stream.readInt();   
   if(version < 9){
     stream.readChar();
     WeatherSystem::legacyWindSpeed = stream.readChar();
     WeatherSystem::legacyWindDirection = stream.readChar();
   }
   
   for ( int j = 0; j < 4; j++ )
      stream.readChar(); // was: different wind in different altitudes
   for ( i = 0; i< 12; i++ )
      stream.readChar(); // dummy

   _resourcemode = stream.readInt();

   for ( i = 0; i < 8; i++ )
      for ( int j = 0; j < 8; j++ )
         alliances[j][i] = stream.readChar();

   int dummy_playername[9];
   for ( i = 0; i< 9; i++ ) {
      player[i].existanceAtBeginOfTurn = stream.readChar();
      stream.readInt(); // dummy
      stream.readInt(); // dummy
      if ( version <= 5 )
         player[i].research.read_struct ( stream );
      else
         player[i].research.read ( stream );

      player[i].ai = (BaseAI*) stream.readInt();
      player[i].stat = Player::tplayerstat ( stream.readChar() );
      stream.readChar(); // dummy
      dummy_playername[i] = stream.readInt();
      player[i].passwordcrc.read ( stream );
      player[i].__dissectionsToLoad = stream.readInt();
      player[i].__loadunreadmessage = stream.readInt();
      player[i].__loadoldmessage = stream.readInt();
      player[i].__loadsentmessage = stream.readInt();
      if ( version >= 3 )
         player[i].ASCversion = stream.readInt();
      else
         player[i].ASCversion = 0;
         
      if ( version >= 9 )
         player[i].cursorPos.read( stream );
   }



   if ( version <= 4 ) {
   //! practically dummy
      loadOldEvents = stream.readInt();
      stream.readInt(); // int loadeventstocome
      stream.readInt(); // int loadeventpassed
   }


   unitnetworkid = stream.readInt();
   levelfinished = stream.readChar();
   network = (pnetwork) stream.readInt();
   bool alliance_names_not_used_any_more[8];
   for ( i = 0; i < 8; i++ )
      alliance_names_not_used_any_more[i] = stream.readInt(); // dummy

   for ( i = 0; i< 8; i++ ) {
      cursorpos.position[i].cx = stream.readWord();
      cursorpos.position[i].sx = stream.readWord();
      cursorpos.position[i].cy = stream.readWord();
      cursorpos.position[i].sy = stream.readWord();
   }

   stream.readInt(); // loadtribute
   __loadunsentmessage = stream.readInt();
   __loadmessages = stream.readInt();

   messageid = stream.readInt();

   if(  version < 2 ) {
      ___loadJournal = stream.readInt();
      ___loadNewJournal = stream.readInt();
   } else {
      ___loadJournal = true;
      ___loadNewJournal = true;
   }

   int exist_humanplayername[9];
   for ( i = 0; i < 8; i++ )
      exist_humanplayername[i] = stream.readInt();
   exist_humanplayername[8] = 0;


   int exist_computerplayername[9];
   for ( i = 0; i < 8; i++ )
      exist_computerplayername[i] = stream.readInt();
   exist_computerplayername[8] = 0;

   supervisorpasswordcrc.read ( stream );

   for ( i = 0; i < 8; i++ )
      alliances_at_beginofturn[i] = stream.readChar();

   stream.readInt(); // was objectcrc = (pobjectcontainercrcs)
   bool load_shareview = stream.readInt();

   continueplaying = stream.readInt();
   __loadreplayinfo =  stream.readInt();
   playerView = stream.readInt();
   lastjournalchange.abstime = stream.readInt();

   for ( i = 0; i< 8; i++ )
      bi_resource[i].read ( stream );

   int preferredfilenames = stream.readInt();

   ellipse = (EllipseOnScreen*) stream.readInt();
   graphicset = stream.readInt();
   gameparameter_num = stream.readInt();

   stream.readInt(); // dummy
   mineralResourcesDisplayed = stream.readInt();
   for ( i = 0; i< 9; i++ )
       player[i].queuedEvents = stream.readInt();

   for ( i = 0; i < 19; i++ )
       stream.readInt();

   int _oldgameparameter[8];
   for ( i = 0; i < 8; i++ )
       _oldgameparameter[i] = stream.readInt();

// return;

/////////////////////
// Here initmap was called
/////////////////////


    if ( ___loadtitle )
       maptitle = stream.readString();

    if ( loadCampaign ) {
       campaign = new Campaign;
       campaign->id = stream.readWord();
       campaign->prevmap = stream.readWord();
       campaign->player = stream.readChar();
       campaign->directaccess = stream.readChar();
       for ( int d = 0; d < 21; d++ )
          stream.readChar(); // dummy
    }

    for ( int w=0; w<9 ; w++ ) {
       if (dummy_playername[w] )
          stream.readString();

       player[w].ai = NULL;


       if ( exist_humanplayername[w] )
          player[w].humanname = stream.readString();

       if ( exist_computerplayername[w] )
          player[w].computername = stream.readString();

    } /* endfor */

    if ( stream.readInt() )
       tribute.read ( stream );

    for ( int aa = 0; aa < 8; aa++ )
       if ( alliance_names_not_used_any_more[aa] ) {
          char* tempname = NULL;
          stream.readpchar ( &tempname );
          delete[] tempname;
       }

    stream.readInt();

    if ( load_shareview ) {
       shareview = new tmap::Shareview;
       shareview->read ( stream );
    } else
       shareview = NULL;

    if ( preferredfilenames ) {
       int p;
       int mapname[8];
       int mapdescription_not_used_any_more[8];
       int savegame[8];
       int savegamedescription_not_used_any_more[8];
       for ( p = 0; p < 8; p++ )
          mapname[p] = stream.readInt();
       for ( p = 0; p < 8; p++ )
          mapdescription_not_used_any_more[p] = stream.readInt();
       for ( p = 0; p < 8; p++ )
          savegame[p] = stream.readInt();
       for ( p = 0; p < 8; p++ )
          savegamedescription_not_used_any_more[p] = stream.readInt();

       for ( int i = 0; i < 8; i++ ) {
          if ( mapname[i] )
             preferredFileNames.mapname[i] = stream.readString ();

          if ( mapdescription_not_used_any_more[i] )
             stream.readString(); // dummy

          if ( savegame[i] )
             preferredFileNames.savegame[i] = stream.readString ();

          if ( savegamedescription_not_used_any_more[i] )
             stream.readString();
       }
    }

    if ( ellipse ) {
       ellipse = new EllipseOnScreen;
       ellipse->read( stream );
    }

    int orggpnum = gameparameter_num;
    gameparameter_num = 0;
    for ( int gp = 0; gp < 8; gp ++ )
       setgameparameter ( GameParameter(gp), _oldgameparameter[gp] );

    for ( int ii = 0 ; ii < orggpnum; ii++ ) {
       int gpar = stream.readInt();
       setgameparameter ( GameParameter(ii), gpar );
    }

    if ( version >= 2 ) {
       archivalInformation.author = stream.readString();
       archivalInformation.description = stream.readString();
       archivalInformation.tags = stream.readString();
       archivalInformation.requirements = stream.readString();
       archivalInformation.modifytime = stream.readInt();
    }

    if ( version >= 4 ) {
       int num = stream.readInt();
       for ( int ii = 0; ii < num; ++ii )
          unitProduction.idsAllowed.push_back ( stream.readInt() );

       for ( int ii = 0; ii < 9; ii++ ) {
          int num = stream.readInt( );
          for ( int i = 0; i < num; i++ ) {
             Player::PlayTime pt;
             pt.turn = stream.readInt();
             pt.date = stream.readInt();
             player[ii].playTime.push_back ( pt );
          }
       }

    }

    if ( version >= 5 ) {
       eventID = stream.readInt();

       int num = stream.readInt();
       for ( int i = 0; i< num; ++i ) {
          Event* ev = new Event ( *this );
          ev->read ( stream );
          events.push_back ( ev );
       }
    }

    if ( version >= 8 )
       randomSeed = stream.readInt();
}


void tmap :: write ( tnstream& stream )
{
   int i;

   stream.writeWord( 0xfffe );
   stream.writeWord( 0xfffc );

   stream.writeInt ( tmapversion );
   stream.writeInt( xsize );
   stream.writeInt( ysize );

   stream.writeWord( xpos );
   stream.writeWord( ypos );
   stream.writeInt (1); // dummy
   stream.writedata ( codeword, 11 );

   stream.writeInt( campaign != NULL);
   stream.writeChar( actplayer );
   stream.writeInt( time.abstime );   
   for  ( i= 0; i < 4; i++ )
      stream.writeChar( 0 );

   for ( i = 0; i< 12; i++ )
      stream.writeChar( 0 ); // dummy

   stream.writeInt( _resourcemode );

   for ( i = 0; i < 8; i++ )
      for ( int j = 0; j < 8; j++ )
         stream.writeChar( alliances[j][i] );

   for ( i = 0; i< 9; i++ ) {
      stream.writeChar( player[i].existanceAtBeginOfTurn );
      stream.writeInt( 1 ); // dummy
      stream.writeInt( 1 ); // dummy
      player[i].research.write ( stream );
      stream.writeInt( player[i].ai != NULL );
      stream.writeChar( player[i].stat );
      stream.writeChar( 0 ); // dummy
      stream.writeInt( 0 );
      player[i].passwordcrc.write ( stream );
      stream.writeInt( !player[i].dissections.empty() );
      stream.writeInt( 1 );
      stream.writeInt( 1 );
      stream.writeInt( 1 );
      stream.writeInt ( player[i].ASCversion );
      player[i].cursorPos.write( stream );
   }

   stream.writeInt( unitnetworkid );
   stream.writeChar( levelfinished );
   stream.writeInt( network != NULL );
   for ( i = 0; i < 8; i++ )
      stream.writeInt( 0 ); // dummy

   for ( i = 0; i< 8; i++ ) {
      stream.writeWord( cursorpos.position[i].cx );
      stream.writeWord( cursorpos.position[i].sx );
      stream.writeWord( cursorpos.position[i].cy );
      stream.writeWord( cursorpos.position[i].sy );
   }

   stream.writeInt( -1 );
   stream.writeInt( 1 );
   stream.writeInt( !messages.empty() );

   stream.writeInt( messageid );

   for ( i = 0; i < 8; i++ )
      stream.writeInt( !player[i].humanname.empty() );

   for ( i = 0; i < 8; i++ )
      stream.writeInt( !player[i].computername.empty() );

   supervisorpasswordcrc.write ( stream );

   for ( i = 0; i < 8; i++ )
      stream.writeChar( alliances_at_beginofturn[i] );

   stream.writeInt( 0 );
   stream.writeInt( shareview != NULL );

   stream.writeInt( continueplaying );
   stream.writeInt( replayinfo != NULL );
   stream.writeInt( playerView );
   stream.writeInt( lastjournalchange.abstime );

   for ( i = 0; i< 8; i++ )
      bi_resource[i].write ( stream );

   stream.writeInt( 1 );
   stream.writeInt( ellipse != NULL );
   stream.writeInt( graphicset );
   stream.writeInt( gameparameter_num );

   stream.writeInt( game_parameter != NULL );
   stream.writeInt( mineralResourcesDisplayed );
   for ( i = 0; i< 9; i++ )
       stream.writeInt( player[i].queuedEvents );

   for ( i = 0; i < 19; i++ )
       stream.writeInt( 0 );

   for ( i = 0; i < 8; i++ )
       stream.writeInt( getgameparameter(GameParameter(i)) );


///////////////////
// second part
//////////////////



   stream.writeString( maptitle );

   if ( campaign ) {
      stream.writeWord( campaign->id );
      stream.writeWord( campaign->prevmap );
      stream.writeChar( campaign->player );
      stream.writeChar( campaign->directaccess );
      for ( int d = 0; d < 21; d++ )
         stream.writeChar(0); // dummy
   }

   for (int w=0; w<8 ; w++ ) {

      if ( !player[w].humanname.empty() )
         stream.writeString ( player[w].humanname );

      if ( !player[w].computername.empty() )
         stream.writeString ( player[w].computername );
   }

   if ( !tribute.empty() ) {
       stream.writeInt ( -1 );
       tribute.write ( stream );
   } else
       stream.writeInt ( 0 );

    stream.writeInt ( 0 );

    if ( shareview )
       shareview->write ( stream );

    int p;
    for ( p = 0; p < 8; p++ )
       stream.writeInt( 1 );

    for ( p = 0; p < 8; p++ )
       stream.writeInt( 0 );

    for ( p = 0; p < 8; p++ )
       stream.writeInt( 1 );

    for ( p = 0; p < 8; p++ )
       stream.writeInt( 0 );

    for ( int k = 0; k < 8; k++ ) {
       stream.writeString ( preferredFileNames.mapname[k] );
       stream.writeString ( preferredFileNames.savegame[k] );
    }


    if ( ellipse )
       ellipse->write( stream );

    for ( int ii = 0 ; ii < gameparameter_num; ii++ )
       stream.writeInt ( game_parameter[ii] );


    stream.writeString ( archivalInformation.author );
    stream.writeString ( archivalInformation.description );
    stream.writeString ( archivalInformation.tags );
    stream.writeString ( archivalInformation.requirements );
    stream.writeInt ( ::time ( &archivalInformation.modifytime ));


    stream.writeInt( unitProduction.idsAllowed.size() );
    for ( int ii = 0; ii < unitProduction.idsAllowed.size(); ++ii )
       stream.writeInt ( unitProduction.idsAllowed[ii] );


    for ( int ii = 0; ii < 9; ii++ ) {
       stream.writeInt( player[ii].playTime.size() );
       for ( Player::PlayTimeContainer::iterator i = player[ii].playTime.begin(); i != player[ii].playTime.end(); ++i ) {
          stream.writeInt( i->turn );
          stream.writeInt( i->date );
       }
    }

    stream.writeInt( eventID );

    stream.writeInt ( events.size());
    for ( Events::iterator i = events.begin(); i != events.end(); ++i )
       (*i)->write( stream );

    stream.writeInt( randomSeed );
}




MapCoordinate& tmap::getCursor()
{
   return player[actplayer].cursorPos;
}


void tmap :: cleartemps( int b, int value )
{
  if ( xsize <= 0 || ysize <= 0)
     return;

  int l = 0;
  for ( int x = 0; x < xsize ; x++)
     for ( int y = 0; y <  ysize ; y++) {

         if (b & 1 )
           field[l].a.temp = value;
         if (b & 2 )
           field[l].a.temp2 = value;
         if (b & 4 )
           field[l].temp3 = value;
         if (b & 8 )
           field[l].temp4 = value;

         l++;
     }
}

void tmap :: allocateFields ( int x, int y )
{
   field = new tfield[x*y];
   for ( int i = 0; i < x*y; i++ )
      field[i].setMap ( this );
   xsize = x;
   ysize = y;
}


void tmap :: calculateAllObjects ( void )
{
   calculateallobjects();
}

pfield  tmap :: getField(int x, int y)
{
   if ((x < 0) || (y < 0) || (x >= xsize) || (y >= ysize))
      return NULL;
   else
      return (   &field[y * xsize + x] );
}

pfield  tmap :: getField(const MapCoordinate& pos )
{
   return getField ( pos.x, pos.y );
}


bool tmap :: isResourceGlobal ( int resource )
{
   if ( _resourcemode == 1 ) { // BI-Mode
      if ( resource == 1 ) // material
         return false;
      else
         if ( resource == 2 )
            return getgameparameter(cgp_globalfuel);
         else
            return true;
   } else {
      /*
      if ( resource == 0 )
         return getgameparameter(cgp_globalenergy);
      if ( resource == 1 )
         return false;
      if ( resource == 2 )
         return getgameparameter(cgp_globalfuel);
      */
      return false;
   }
}

int tmap :: getgameparameter ( GameParameter num )
{
  if ( game_parameter && num < gameparameter_num ) {
     return game_parameter[num];
  } else
     if ( num < gameparameternum )
        return gameparameterdefault[ num ];
     else
        return 0;
}

void tmap :: setgameparameter ( GameParameter num, int value )
{
   if ( game_parameter ) {
     if ( num < gameparameter_num )
        game_parameter[num] = value;
     else {
        int* oldparam = game_parameter;
        game_parameter = new int[num+1];
        for ( int i = 0; i < gameparameter_num; i++ )
           game_parameter[i] = oldparam[i];
        for ( int j = gameparameter_num; j < num; j++ )
           if ( j < gameparameternum )
              game_parameter[j] = gameparameterdefault[j];
           else
              game_parameter[j] = 0;
        game_parameter[num] = value;
        gameparameter_num = num + 1;
        delete[] oldparam;
     }
   } else {
       game_parameter = new int[num+1];
       for ( int j = 0; j < num; j++ )
          if ( j < gameparameternum )
             game_parameter[j] = gameparameterdefault[j];
          else
             game_parameter[j] = 0;
       game_parameter[num] = value;
       gameparameter_num = num + 1;
   }
}

void tmap :: setupResources ( void )
{
   for ( int n = 0; n< 8; n++ ) {
      bi_resource[n].energy = 0;
      bi_resource[n].material = 0;
      bi_resource[n].fuel = 0;

     #ifdef sgmain

      for ( Player::BuildingList::iterator i = player[n].buildingList.begin(); i != player[n].buildingList.end(); i++ )
         for ( int r = 0; r < 3; r++ )
            if ( isResourceGlobal( r )) {
               bi_resource[n].resource(r) += (*i)->actstorage.resource(r);
               (*i)->actstorage.resource(r) = 0;
            }
     #endif
   }
}



DI_Color tmap :: Player :: getColor()
{
   switch ( player ) {
      case 0: return DI_Color( 0xe0, 0, 0 );
      case 1: return DI_Color( 0, 0x71, 0xdb );
      case 2: return DI_Color( 0xbc, 0xb3, 0 );
      case 3: return DI_Color( 0, 0xaa, 0 );
      case 4: return DI_Color( 0xbc, 0, 0 );
      case 5: return DI_Color( 0xb2, 0, 0xb2 );
      case 6: return DI_Color( 0,0, 0xaa );
      case 7: return DI_Color( 0xbc, 0x67, 0 );
      case 8: return DI_Color( 0xaa, 0xaa, 0xaa );
   };
   return DI_Color(0, 0, 0);
}

const ASCString& tmap :: Player :: getName( )
{
   static ASCString off = "off";
   switch ( stat ) {
     case 0: return humanname;
     case 1: return computername;
     default: return off;
   }
}


const ASCString& tmap :: getPlayerName ( int playernum )
{
   if ( playernum >= 8 )
      playernum /= 8;

   return player[playernum].getName();
}



int tmap :: eventpassed( int saveas, int action, int mapid )
{
   return eventpassed ( (action << 16) | saveas, mapid );
}



int tmap :: eventpassed( int id, int mapid )
{
   return 0;
}


Vehicle* tmap :: getUnit ( Vehicle* eht, int nwid )
{
   if ( !eht )
      return NULL;
   else {
      if ( eht->networkid == nwid )
         return eht;
      else
         for ( int i = 0; i < 32; i++ )
            if ( eht->loading[i] )
               if ( eht->loading[i]->networkid == nwid )
                  return eht->loading[i];
               else {
                  Vehicle* ld = getUnit ( eht->loading[i], nwid );
                  if ( ld )
                     return ld;
               }
      return NULL;
   }
}

Vehicle* tmap :: getUnit ( int nwid )
{
   VehicleLookupCache::iterator i = vehicleLookupCache.find( nwid );
   if ( i != vehicleLookupCache.end() )
      return i->second;


   for ( int p = 0; p < 9; p++ )
      for ( Player::VehicleList::iterator i = player[p].vehicleList.begin(); i != player[p].vehicleList.end(); i++ )
         if ( (*i)->networkid == nwid ) {
            displaymessage("warning: id not registered in VehicleLookupCache!",1);
            return *i;

         }

   return NULL;
}


Vehicle* tmap :: getUnit ( int x, int y, int nwid )
{
   pfield fld  = getField ( x, y );
   if ( !fld )
      return NULL;

   if ( !fld->vehicle )
      if ( fld->building ) {
         for ( int i = 0; i < 32; i++ ) {
            Vehicle* ld = getUnit ( fld->building->loading[i], nwid );
            if ( ld )
               return ld;
         }
         return NULL;
      } else
         return NULL;
   else
      if ( fld->vehicle->networkid == nwid )
         return fld->vehicle;
      else
         return getUnit ( fld->vehicle, nwid );
}

ContainerBase* tmap::getContainer ( int nwid )
{
   if ( nwid > 0 )
      return getUnit(nwid);
   else {
      int x = (-nwid) & 0xffff;
      int y = (-nwid) >> 16;
      return getfield(x,y)->building;
   }
}

bool tmap :: compareResources( tmap* replaymap, int player, ASCString* log )
{
  #ifdef sgmain   
   ASCString s;
   bool diff  = false;
   for ( int r = 0; r < 3; ++r ) {
      if ( isResourceGlobal( r )) {
         if ( bi_resource[player].resource(r) != replaymap-> bi_resource[player].resource(r) ) {
            diff = true;
            if ( log ) {
               s.format ( "Global resource mismatch: %d %s available after replay, but %d available in actual map\n", replaymap-> bi_resource[player].resource(r), resourceNames[r], bi_resource[player].resource(r) );
               *log += s;
            }
         }
      } else {
         GetConnectedBuildings::BuildingContainer cb;
         for ( Player::BuildingList::iterator b = this->player[player].buildingList.begin(); b != this->player[player].buildingList.end(); ++b ) {
            Building* b1 = *b;
            ContainerBase* b2 = replaymap->getContainer( b1->getIdentification() );
            if ( !b1 || !b2 ) {
               if ( log ) {
                  s.format ( "Building missing! \n");
                  *log += s;
               }
            } else {
               if ( find ( cb.begin(), cb.end(), b1 ) == cb.end()) {
                  int ab1 = b1->getResource( maxint, r, true);
                  int ab2 = b2->getResource( maxint, r, true);
                  if ( ab1 != ab2 ) {
                     diff = true;
                     if ( log ) {
                        s.format ( "Building (%d,%d) resource mismatch: %d %s available after replay, but %d available in actual map\n", b1->getPosition().x, b1->getPosition().y, ab1, resourceNames[r], ab2 );
                        *log += s;
                     }
                  }
                  cb.push_back ( b1 );
                  GetConnectedBuildings::BuildingContainer cbl;
                  GetConnectedBuildings gcb ( cbl, b1->getMap(), r );
                  gcb.start ( b1->getPosition().x, b1->getPosition().y );
                  cb.insert ( cb.end(), cbl.begin(), cbl.end() );
               }
            }
         }
      }
      for ( Player::VehicleList::iterator v = this->player[player].vehicleList.begin(); v != this->player[player].vehicleList.end(); ++v ) {
         Vehicle* v1 = *v;
         Vehicle* v2 = replaymap->getUnit( v1->networkid );
         if ( !v1 || !v2 ) {
            if ( log ) {
               s.format ( "Vehicle missing! \n");
               *log += s;
            }
         } else {
            int av1 = v1->getResource( maxint, r, true );
            int av2 = v2->getResource( maxint, r, true );
            if ( av1 != av2 ) {
               diff = true;
               if ( log ) {
                  s.format ( "Vehicle (%d,%d) resource mismatch: %d %s available after replay, but %d available in actual map\n", v1->getPosition().x, v1->getPosition().y, av2, resourceNames[r], av1 );
                  *log += s;
               }
            }
         }
      }

   }
   if ( this->player[player].vehicleList.size() != replaymap->player[player].vehicleList.size() ) {
      diff = true;
      if ( log ) {
         s.format ( "The number of units differ. Replay: %d ; actual map: %d", replaymap->player[player].vehicleList.size(), this->player[player].vehicleList.size());
         *log += s;
      }
   }
   if ( this->player[player].buildingList.size() != replaymap->player[player].buildingList.size() ) {
      diff = true;
      if ( log ) {
         s.format ( "The number of buildings differ. Replay: %d ; actual map: %d", replaymap->player[player].buildingList.size(), this->player[player].buildingList.size());
         *log += s;
      }
   }

   if ( this->player[player].research.progress != replaymap->player[player].research.progress ) {
      diff = true;
      if ( log ) {
         s.format ( "Research points mismatch! Replay: %d ; actual map: %d", replaymap->player[player].research.progress, this->player[player].research.progress);
         *log += s;
      }
   }

   sort ( this->player[player].research.developedTechnologies.begin(), this->player[player].research.developedTechnologies.end() );
   sort ( replaymap->player[player].research.developedTechnologies.begin(), replaymap->player[player].research.developedTechnologies.end() );
   if ( replaymap->player[player].research.developedTechnologies.size() != this->player[player].research.developedTechnologies.size() ) {
      diff = true;
      if ( log ) {
         s.format ( "Number of developed technologies differ !\n" );
         *log += s;
      }
   } else {
      for ( int i = 0; i < replaymap->player[player].research.developedTechnologies.size(); ++i )
         if ( replaymap->player[player].research.developedTechnologies[i] != this->player[player].research.developedTechnologies[i] ) {
            diff = true;
            if ( log ) {
               s.format ( "Different technologies developed !\n" );
               *log += s;
            }
         }
   }

   for ( Player::BuildingList::iterator b = this->player[player].buildingList.begin(); b != this->player[player].buildingList.end(); ++b ) {
      Building* b1 = *b;
      Building* b2 = dynamic_cast<Building*>(replaymap->getContainer( b1->getIdentification() ));
      if ( !b1 || !b2 ) {
         if ( log ) {
            s.format ( "Building missing! \n");
            *log += s;
         }
      } else {
         bool mismatch = false;
         for ( int i = 0; i < 32; ++i )
            if ( b1->production[i] ) {
               bool found = false;
               for ( int j = 0; j < 32; ++j)
                  if ( b2->production[j] == b1->production[i] )
                     found = true;
               if ( !found)
                  mismatch = true;
            }

         for ( int j = 0; j < 32; ++j )
            if ( b2->production[j] ) {
               bool found = false;
               for ( int i = 0; i < 32; ++i)
                  if ( b1->production[i] == b2->production[j] )
                     found = true;
               if ( !found)
                  mismatch = true;
            }

         if ( mismatch ) {
            diff = true;
            if ( log ) {
               s.format ( "Building (%d,%d) production line mismatch !\n", b1->getPosition().x, b1->getPosition().y );
               *log += s;
            }
         }
      }
   }

   return diff;
   #else
   return false;
   #endif
}


void tmap::endTurn()
{
   player[actplayer].ASCversion = getNumericVersion();
   Player::PlayTime pt;
   pt.turn = time.turn();
   ::time ( &pt.date );
   player[actplayer].playTime.push_back ( pt );

   for ( tmap::Player::BuildingList::iterator b = player[actplayer].buildingList.begin(); b != player[actplayer].buildingList.end(); ++b )
      (*b)->endTurn();

   tmap::Player::VehicleList toRemove;
   for ( tmap::Player::VehicleList::iterator v = player[actplayer].vehicleList.begin(); v != player[actplayer].vehicleList.end(); ++v ) {
      Vehicle* actvehicle = *v;

      // Bei Aenderungen hier auch die Windanzeige dashboard.PAINTWIND aktualisieren !!!

      if (( actvehicle->height >= chtieffliegend )   &&  ( actvehicle->height <= chhochfliegend ) && ( getfield(actvehicle->xpos,actvehicle->ypos)->vehicle == actvehicle)) {
         if ( getmaxwindspeedforunit ( actvehicle ) < weatherSystem->getCurrentWindSpeed()*maxwindspeed ){
            ASCString ident = "The unit " + (*v)->getName() + " at position ("+strrr((*v)->getPosition().x)+"/"+strrr((*v)->getPosition().y)+") crashed because of the strong wind";
            new Message ( ident, actmap, 1<<(*v)->getOwner());
            toRemove.push_back ( *v );
         } else {

            int j = actvehicle->getTank().fuel - actvehicle->typ->fuelConsumption * nowindplanefuelusage;

            if ( actvehicle->height <= chhochfliegend ) {
               int mo = actvehicle->typ->movement[log2(actvehicle->height)];
               if ( mo )
                  j -= ( actvehicle->getMovement() * 64 / mo)
                       * (weatherSystem->getCurrentWindSpeed() * maxwindspeed / 256 ) * actvehicle->typ->fuelConsumption / ( minmalq * 64 );
               else
                  j -= (weatherSystem->getCurrentWindSpeed() * maxwindspeed / 256 ) * actvehicle->typ->fuelConsumption / ( minmalq * 64 );
            }
           //          movement * 64        windspeed * maxwindspeed         fuelConsumption
           // j -=   ----------------- *  ----------------------------- *   -----------
           //          typ->movement                 256                       64 * 8
           //
           //

           //gek?rzt:
           //
           //             movement            windspeed * maxwindspeed
           // j -= --------------------- *  ----------------------------   * fuelConsumption
           //           typ->movement             256   *      8
           //
           //
           //
           // Falls eine vehicle sich nicht bewegt hat, bekommt sie soviel Sprit abgezogen, wie sie zum zur?cklegen der Strecke,
           // die der Wind pro Runde zur?ckgelegt hat, fuelConsumptionen w?rde.
           // Wenn die vehicle sich schon bewegt hat, dann wurde dieser Abzug schon beim movement vorgenommen, so da�er hier nur

           // noch fuer das ?briggebliebene movement stattfinden mu�
           //


            if (j < 0) {
               ASCString ident = "The unit " + (*v)->getName() + " at position ("+strrr((*v)->getPosition().x);
               ident += ASCString("/")+strrr((*v)->getPosition().y)+") crashed due to lack of fuel";
               new Message ( ident, actmap, 1<<(*v)->getOwner());
               toRemove.push_back ( *v );
               // logtoreplayinfo( rpl_removeunit, actvehicle->getPosition().x, actvehicle->getPosition().y, actvehicle->networkid );
            } else {
               // logtoreplayinfo( rpl_refuel2, actvehicle->getPosition().x, actvehicle->getPosition().y, actvehicle->networkid, 1002, j, actvehicle->tank.fuel );
               actvehicle->getResource( actvehicle->getTank().fuel - j, Resources::Fuel, false);
            }
         }
      }

      if ( actvehicle )
         actvehicle->endTurn();

   }

   for ( tmap::Player::VehicleList::iterator v = toRemove.begin(); v != toRemove.end(); v++ )
      delete *v;

   checkunitsforremoval();

}


void tmap::endRound()
{
    actplayer = 0;
    time.set ( time.turn()+1, 0 );

    for ( int y = 0; y < ysize; ++y )
       for ( int x = 0; x < xsize; ++x )
           getField(x,y)->endRound( time.turn() );

    for (int i = 0; i <= 7; i++)
       if (player[i].exist() ) {

          for ( tmap::Player::VehicleList::iterator j = player[i].vehicleList.begin(); j != player[i].vehicleList.end(); j++ )
             (*j)->endRound();

          typedef PointerList<Building::Work*> BuildingWork;
          BuildingWork buildingWork;

          for ( tmap::Player::BuildingList::iterator j = player[i].buildingList.begin(); j != player[i].buildingList.end(); j++ ) {
             Building::Work* w = (*j)->spawnWorkClasses( false );
             if ( w )
                buildingWork.push_back ( w );
          }

          bool didSomething;
          do {
             didSomething = false;
             for ( BuildingWork::iterator j = buildingWork.begin(); j != buildingWork.end(); j++ )
                if ( ! (*j)->finished() ) 
    //weatherSystem->update(time);
                   if ( (*j)->run() )
                      didSomething = true;

          } while ( didSomething );
          doresearch( this, i );
       }

    if ( getgameparameter( cgp_objectGrowthMultiplier) > 0 )
       objectGrowth();
}

#include "libs/rand/rand_r.h"
#include "libs/rand/rand_r.c"

int tmap::random( int max )
{
   return rand_r( &randomSeed ) % max;
}

void tmap::objectGrowth()
{
   typedef vector< pair<pfield,int> > NewObjects;
   NewObjects newObjects;
   for ( int y = 0; y < ysize; ++y )
      for ( int x = 0; x < xsize; ++x ) {
          pfield fld = getField( x, y );
          for ( tfield::ObjectContainer::iterator i = fld->objects.begin(); i != fld->objects.end(); ++i)
             if ( i->typ->growthRate > 0 )
                for ( int d = 0; d < 6; ++d ) {
                   pfield fld2 = getField ( getNeighbouringFieldCoordinate( MapCoordinate(x,y), d ));
                   if ( fld2 && (!fld2->vehicle || fld2->vehicle->height >= chtieffliegend) && !fld2->building ) 
                      if ( fld2->objects.empty() || getgameparameter( cgp_objectGrowOnOtherObjects ) > 0 ) {
                        double d = i->typ->growthRate * getgameparameter( cgp_objectGrowthMultiplier) / 100;
                        if ( d > 0 ) {
                           if ( d > 0.9 )
                              d = 0.9;

                           int p = static_cast<int>(std::ceil ( double(1) / d)) ;
                           if ( p > 1 )
                              if ( random ( p ) == 1 )
                                 if ( i->typ->fieldModification[fld2->getweather()].terrainaccess.accessible( fld2->bdt) > 0 )
                                    newObjects.push_back( make_pair( fld2, i->typ->id ));
                        }
                   }
                }
      }

   for ( NewObjects::iterator i = newObjects.begin(); i != newObjects.end(); ++i )
      i->first->addobject ( getobjecttype_byid( i->second ));
}


bool tmap::nextPlayer()
{
   int runde = 0;
   do {
      actplayer++;
      time.set ( time.turn(), 0 );
      if (actplayer > 7) {
         endRound();
         runde++;
         newRound();
      }
  
      if ( !player[actplayer].exist() )
         if ( replayinfo )
            if ( replayinfo->guidata[actplayer] ) {
               delete replayinfo->guidata[actplayer];
               replayinfo->guidata[actplayer] = NULL;
            }

   }  while ( (!player[actplayer].exist() || player[actplayer].stat == Player::off)  && (runde <= 2)  );
   return runde <= 2;
}

tmap :: ~tmap ()
{
   __mapDestruction = true;

   if ( field )

      for ( int l=0 ;l < xsize * ysize ; l++ ) {
         if ( (field[l].bdt & getTerrainBitType(cbbuildingentry)).any() )
            delete field[l].building;


         Vehicle* aktvehicle = field[l].vehicle;
         if ( aktvehicle )
            delete aktvehicle;

      } /* endfor */

   int i;
   for ( i = 0; i <= 8; i++) {
      if ( player[i].ai ) {
         delete player[i].ai;
         player[i].ai = NULL;
      }
   }


   if ( shareview ) {
      delete shareview;
      shareview = NULL;
   }

   if ( replayinfo ) {
      delete replayinfo;
      replayinfo = NULL;
   }

   if ( game_parameter ) {
      delete[] game_parameter;
      game_parameter = NULL;
   }

   delete weatherSystem;

   if ( field ) {
      delete[] field;
      field = NULL;
   }

   if ( actmap == this )
      actmap = NULL;

}

/*
gamemap :: ResourceTribute :: tresourcetribute ( )
{
   for ( int a = 0; a < 3; a++ )
      for ( int b = 0; b < 8; b++ )
         for ( int c = 0; c < 8; c++ ) {
            avail.resource[a][b][c] = 0;
            paid.resource[a][b][c] = 0;
         }
}
*/

bool tmap :: ResourceTribute :: empty ( )
{
   for (int i = 0; i < 8; i++)
      for (int j = 0; j < 8; j++)
         for (int k = 0; k < 3; k++) {
            if ( avail[i][j].resource(k) )
               return false;
            if ( paid[i][j].resource(k) )
               return false;
         }

   return true;
}

const int tributeVersion = 1;  // we are counting backwards, -2 is newer than -1

void tmap :: ResourceTribute :: read ( tnstream& stream )
{
   int version = stream.readInt();
   bool noVersion;

   if ( -version >= tributeVersion ) {
      for ( int a = 0; a < 8; ++a )
         for ( int b = 0; b < 8; ++b )
            payStatusLastTurn[a][b].read( stream );
      noVersion = false;
   } else
      noVersion = true;


   for ( int a = 0; a < 3; a++ )
      for ( int b = 0; b < 8; b++ )
         for ( int c = 0; c < 8; c++ ) {
             if ( noVersion ) {
                avail[b][c].resource(a) = version;
                noVersion = false;
             } else
                avail[b][c].resource(a) = stream.readInt();
          }

   for ( int a = 0; a < 3; a++ )
      for ( int b = 0; b < 8; b++ )
         for ( int c = 0; c < 8; c++ )
             paid[b][c].resource(a) = stream.readInt();
}

void tmap :: ResourceTribute :: write ( tnstream& stream )
{
   stream.writeInt ( -tributeVersion );
   for ( int a = 0; a < 8; a++ )
      for ( int b = 0; b < 8; b++ )
         payStatusLastTurn[a][b].write( stream );

   for ( int a = 0; a < 3; a++ )
      for ( int b = 0; b < 8; b++ )
         for ( int c = 0; c < 8; c++ )
             stream.writeInt ( avail[b][c].resource(a) );


   for ( int a = 0; a < 3; a++ )
      for ( int b = 0; b < 8; b++ )
         for ( int c = 0; c < 8; c++ )
             stream.writeInt ( paid[b][c].resource(a) );
}



int  tmap::resize( int top, int bottom, int left, int right )  // positive: larger
{
  if ( !top && !bottom && !left && !right )
     return 0;

  if ( -(top + bottom) > ysize )
     return 1;

  if ( -(left + right) > xsize )
     return 2;

  if ( bottom & 1 || top & 1 )
     return 3;

  int ox1, oy1, ox2, oy2;

  if ( top < 0 ) {
     for ( int x = 0; x < xsize; x++ )
        for ( int y = 0; y < -top; y++ )
           getField(x,y)->deleteeverything();

     oy1 = -top;
  } else
     oy1 = 0;

  if ( bottom < 0 ) {
     for ( int x = 0; x < xsize; x++ )
        for ( int y = ysize+bottom; y < ysize; y++ )
           getField(x,y)->deleteeverything();

     oy2 = ysize + bottom;
  } else
     oy2 = ysize;
   
  if ( left < 0 ) {
     for ( int x = 0; x < -left; x++ )
        for ( int y = 0; y < ysize; y++ )
           getField(x,y)->deleteeverything();
     ox1 = -left;
  } else
     ox1 = 0;

  if ( right < 0 ) {
     for ( int x = xsize+right; x < xsize; x++ )
        for ( int y = 0; y < ysize; y++ )
           getField(x,y)->deleteeverything();
     ox2 = xsize + right;
  } else
     ox2 = xsize;

  for (int s = 0; s < 9; s++)
     for ( tmap::Player::BuildingList::iterator i = player[s].buildingList.begin(); i != player[s].buildingList.end(); i++ )
        (*i)->unchainbuildingfromfield();


  int newx = xsize + left + right;
  int newy = ysize + top + bottom;

  pfield newfield = new tfield [ newx * newy ];
  for ( int i = 0; i < newx * newy; i++ )
     newfield[i].setMap ( this );

  int x;
  for ( x = ox1; x < ox2; x++ )
     for ( int y = oy1; y < oy2; y++ ) {
        pfield org = getField ( x, y );
        pfield dst = &newfield[ (x + left) + ( y + top ) * newx];
        *dst = *org;
     }

  tfield defaultfield;
  defaultfield.setMap ( this );
  defaultfield.typ = getterraintype_byid ( 30 )->weather[0];

  for ( x = 0; x < left; x++ )
     for ( int y = 0; y < newy; y++ )
        newfield[ x + y * newx ] = defaultfield;

  for ( x = xsize + left; x < xsize + left + right; x++ )
     for ( int y = 0; y < newy; y++ )
        newfield[ x + y * newx ] = defaultfield;


  int y;
  for ( y = 0; y < top; y++ )
     for ( int x = 0; x < newx; x++ )
        newfield[ x + y * newx ] = defaultfield;

  for ( y = ysize + top; y < ysize + top + bottom; y++ )
     for ( int x = 0; x < newx; x++ )
        newfield[ x + y * newx ] = defaultfield;

  calculateallobjects();

  for ( int p = 0; p < newx*newy; p++ )
     newfield[p].setparams();

  delete[] field;
  field = newfield;
  xsize = newx;
  ysize = newy;


  for (int s = 0; s < 9; s++)
     for ( tmap::Player::BuildingList::iterator i = player[s].buildingList.begin(); i != player[s].buildingList.end(); i++ ) {
        MapCoordinate mc = (*i)->getEntry();
        mc.x += left;
        mc.y += top;
        (*i)->chainbuildingtofield ( mc );
     }

  for (int s = 0; s < 9; s++)
     for ( tmap::Player::VehicleList::iterator i = player[s].vehicleList.begin(); i != player[s].vehicleList.end(); i++ ) {
        (*i)->xpos += left;
        (*i)->ypos += top;
     }


  /*
  if (xpos + idisplaymap.getscreenxsize() > xsize)
     xpos = xsize - idisplaymap.getscreenxsize() ;
  if (ypos + idisplaymap.getscreenysize()  > ysize)
     ypos = ysize - idisplaymap.getscreenysize() ;
   */

  return 0;
}

bool tmap::Player::exist()
{
  return !(buildingList.empty() && vehicleList.empty());
}

pterraintype tmap :: getterraintype_byid ( int id )
{
   return terrainTypeRepository.getObject_byID ( id );
}

pobjecttype tmap :: getobjecttype_byid ( int id )
{
   return objectTypeRepository.getObject_byID ( id );
}

pvehicletype tmap :: getvehicletype_byid ( int id )
{
   return vehicleTypeRepository.getObject_byID ( id );
}

pbuildingtype tmap :: getbuildingtype_byid ( int id )
{
   return buildingTypeRepository.getObject_byID ( id );
}

const Technology* tmap :: gettechnology_byid ( int id )
{
   return technologyRepository.getObject_byID ( id );
}


pterraintype tmap :: getterraintype_bypos ( int pos )
{
   return terrainTypeRepository.getObject_byPos ( pos );
}

pobjecttype tmap :: getobjecttype_bypos ( int pos )
{
   return objectTypeRepository.getObject_byPos ( pos );
}

pvehicletype tmap :: getvehicletype_bypos ( int pos )
{
   return vehicleTypeRepository.getObject_byPos ( pos );
}

pbuildingtype tmap :: getbuildingtype_bypos ( int pos )
{
   return buildingTypeRepository.getObject_byPos ( pos );
}

const Technology* tmap :: gettechnology_bypos ( int pos )
{
   return technologyRepository.getObject_byPos ( pos );
}

int tmap :: getTerrainTypeNum ( )
{
   return  terrainTypeRepository.getNum();
}

int tmap :: getObjectTypeNum ( )
{
   return  objectTypeRepository.getNum();
}

int tmap :: getVehicleTypeNum ( )
{
   return  vehicleTypeRepository.getNum();
}

int tmap :: getBuildingTypeNum ( )
{
   return  buildingTypeRepository.getNum();
}

int tmap :: getTechnologyNum ( )
{
   return  technologyRepository.getNum();
}

void tmap :: startGame ( )
{
   time.set ( 1, 0 );

   for ( int j = 0; j < 8; j++ )
      player[j].queuedEvents = 1;

   levelfinished = false;
   network = NULL;
   int num = 0;
   int cols[72];

   memset ( cols, 0, sizeof ( cols ));
   int i;
   for ( i = 0; i < 8 ; i++) {
      if ( player[i].exist() ) {
         num++;
         cols[ i * 8 ] = 1;
      } else
         cols[ i * 8 ] = 0;

      cursorpos.position[ i ].sx = 0;
      cursorpos.position[ i ].sy = 0;

   }

   i = 0;                                                                        
   int sze = xsize * ysize;
   do {
      if ( field[i].vehicle ) 
         if ( cols[ field[i].vehicle->color] ) {
            cursorpos.position[ field[i].vehicle->color / 8 ].cx = field[i].vehicle->xpos;
            cursorpos.position[ field[i].vehicle->color / 8 ].cy = field[i].vehicle->ypos;
            num--;
            cols[ field[i].vehicle->color] = 0;
         }

      if ( field[i].building && field[i].building->color < 64 ) 
         if ( cols[ field[i].building->color] ) {
            cursorpos.position[ field[i].building->color / 8 ].cx = field[i].building->getEntry().x;
            cursorpos.position[ field[i].building->color / 8 ].cy = field[i].building->getEntry().y;
            num--;
            cols[ field[i].building->color] = 0;
         }
      i++;
   } while ( num   &&   i <= sze ); /* enddo */


   for ( int n = 0; n< 8; n++ ) {
      bi_resource[n].energy = 0;
      bi_resource[n].material = 0;
      bi_resource[n].fuel = 0;
   }


   #ifndef karteneditor
   actplayer = -1;
   #else
   actplayer = 0;
   #endif
   newRound();
} 

bool tmap::UnitProduction::check ( int id )
{
   for ( tmap::UnitProduction::IDsAllowed::iterator i = idsAllowed.begin(); i != idsAllowed.end(); i ++ )
      if( *i == id )
         return true;

    return false;
}

VisibilityStates tmap::getInitialMapVisibility( int player )
{
   VisibilityStates c = VisibilityStates( getgameparameter ( cgp_initialMapVisibility ));

   if ( this->player[player].ai ) {
      if ( this->player[player].ai->isRunning() ) {
         if ( c < this->player[player].ai->getVision() )
            c = this->player[player].ai->getVision();
      } else
         // this is a hack to make the replays of the AI work
         if ( c < visible_ago )
            c = visible_ago;
   }
   return c;
}



void tmap::operator= ( const tmap& map )
{
  throw ASCmsgException ( "tmap::operator= undefined");
}


tmap::Shareview :: Shareview ( const tmap::Shareview* org )
{
   memcpy ( mode, org->mode, sizeof ( mode ));
   recalculateview = org->recalculateview;
}

tmap::Shareview :: Shareview ( void )
{
   recalculateview = 0;
   for ( int i = 0; i < 8; i++ )
      for ( int j = 0; j< 8; j++ )
         mode[i][j] = false;
}


void tmap::Shareview :: read ( tnstream& stream )
{
   for ( int i = 0; i < 8; i++ )
      for ( int j =0; j < 8; j++ )
         mode[i][j] = stream.readChar();
   recalculateview = stream.readInt();
}

void tmap::Shareview :: write( tnstream& stream )
{
   for ( int i = 0; i < 8; i++ )
      for ( int j =0; j < 8; j++ )
         stream.writeChar( mode[i][j] );
   stream.writeInt( recalculateview );
}


void AiThreat :: write ( tnstream& stream )
{
   const int version = 1000;
   stream.writeInt ( version );
   stream.writeInt ( threatTypes );
   for ( int i = 0; i < threatTypes; i++ )
      stream.writeInt ( threat[i] );
}

void AiThreat:: read ( tnstream& stream )
{

   int version = stream.readInt();
   if ( version == 1000 ) {
      threatTypes = stream.readInt();
      for ( int i = 0; i < threatTypes; i++ )
         threat[i] = stream.readInt();
   }
}


void AiValue :: write ( tnstream& stream )
{
   const int version = 2000;
   stream.writeInt ( version );
   stream.writeInt ( value );
   stream.writeInt ( addedValue );
   threat.write ( stream );
   stream.writeInt ( valueType );
}

void AiValue:: read ( tnstream& stream )
{
   int version = stream.readInt();
   if ( version == 2000 ) {
      value = stream.readInt (  );
      addedValue= stream.readInt (  );
      threat.read ( stream );
      valueType = stream.readInt (  );
   }
}

const int aiParamVersion = 3002;

void AiParameter::write ( tnstream& stream )
{
   stream.writeInt ( aiParamVersion );
   stream.writeInt ( lastDamage );
   stream.writeInt ( damageTime.abstime );
   stream.writeInt ( dest.x );
   stream.writeInt ( dest.y );
   stream.writeInt ( dest.getNumericalHeight() );
   stream.writeInt ( dest_nwid );
   stream.writeInt ( data );
   AiValue::write( stream );
   stream.writeInt ( task );
   stream.writeInt ( jobPos );
   stream.writeInt ( jobs.size() );
   for ( int i = 0; i < jobs.size(); i++ )
      stream.writeInt( jobs[i] );
   stream.writeInt ( resetAfterJobCompletion );
}

void AiParameter::read ( tnstream& stream )
{
   int version = stream.readInt();
   if ( version >= 3000 && version <= aiParamVersion ) {
      lastDamage = stream.readInt();
      damageTime.abstime = stream.readInt();
      int x = stream.readInt();
      int y = stream.readInt();
      int z = stream.readInt();
      dest.setnum ( x, y, z );
      dest_nwid = stream.readInt();
      data = stream.readInt();
      AiValue::read( stream );
      task = (Task) stream.readInt();
      if ( version == 3000 ) {
         jobs.clear();
         jobs.push_back ( Job( stream.readInt() ));
      } else {
         jobPos = stream.readInt();
         int num = stream.readInt();
         jobs.clear();
         for ( int i = 0; i < num; i++ )
            jobs.push_back ( Job( stream.readInt() ));
      }
      if ( version >= 3002 )
         resetAfterJobCompletion = stream.readInt();
      else
         resetAfterJobCompletion = false;
   }
}

void AiThreat :: reset ( void )
{
   for ( int i = 0; i < aiValueTypeNum; i++ )
      threat[i] = 0;
}

AiParameter :: AiParameter ( Vehicle* _unit ) : AiValue ( log2( _unit->height ))
{
   reset( _unit );
}

void AiParameter :: resetTask ( )
{
   dest.setnum ( -1, -1, -1 );
   dest_nwid = -1;
   task = tsk_nothing;
}

void AiParameter::addJob ( Job j, bool front )
{
   if ( front )
      jobs.insert ( jobs.begin(), j );
   else
      jobs.push_back ( j );
}

void AiParameter::setJob ( const JobList& jobs )
{
   this->jobs = jobs;
}

void AiParameter::setJob ( Job j )
{
   int pos = 0;
   for ( JobList::iterator i = jobs.begin(); i != jobs.end(); ++i, ++pos )
      if ( *i == j ) {
         jobPos = pos;
         return;
      }

   addJob ( j, true );
}


bool AiParameter::hasJob ( AiParameter::Job j )
{
   return find ( jobs.begin(), jobs.end(), j ) != jobs.end();
}


void AiParameter :: reset ( Vehicle* _unit )
{
   unit = _unit;
   AiValue::reset ( log2( _unit->height ) );

   clearJobs();
   resetTask();
   resetAfterJobCompletion = false;
}

void AiParameter :: setNextJob()
{
   jobPos++;
}

void AiParameter :: restartJobs()
{
   jobPos = 0;
}

void AiParameter :: clearJobs()
{
   jobs.clear();
   jobPos = 0;
}




tmap :: ReplayInfo :: ReplayInfo ( void )
{
   for (int i = 0; i < 8; i++) {
      guidata[i] = NULL;
      map[i] = NULL;
   }
   actmemstream = NULL;
   stopRecordingActions = 0;
}

void tmap :: ReplayInfo :: read ( tnstream& stream )
{
   bool loadgui[8];
   bool loadmap[8];

   for ( int i = 0; i < 8; i++ )
      loadgui[i] = stream.readInt();

   for ( int i = 0; i < 8; i++ )
      loadmap[i] = stream.readInt();

   stream.readInt(); // was: actmemstream

   for ( int i = 0; i < 8; i++ ) {
      if ( loadgui[i] ) {
         guidata[i] = new tmemorystreambuf;
         guidata[i]->readfromstream ( &stream );
      } else
         guidata[i] = NULL;

      if ( loadmap[i] ) {
         map[i] = new tmemorystreambuf;
         map[i]->readfromstream ( &stream );
      } else
         map[i] = NULL;
   }

   actmemstream = NULL;
}

void tmap :: ReplayInfo :: write ( tnstream& stream )
{
   for ( int i = 0; i < 8; i++ )
      stream.writeInt ( guidata[i] != NULL );

   for ( int i = 0; i < 8; i++ )
      stream.writeInt ( map[i] != NULL );

   stream.writeInt ( actmemstream != NULL );

   for ( int i = 0; i < 8; i++ ) {
      if ( guidata[i] )
         guidata[i]->writetostream ( &stream );

      if ( map[i] )
         map[i]->writetostream ( &stream );
   }
}


tmap :: ReplayInfo :: ~ReplayInfo ( )
{
   for (int i = 0; i < 8; i++)  {
      if ( guidata[i] ) {
         delete guidata[i];
         guidata[i] = NULL;
      }
      if ( map[i] ) {
         delete map[i];
         map[i] = NULL;
      }
  }
  if ( actmemstream ) {
     delete actmemstream ;
     actmemstream = NULL;
  }
}



const int gameparameterdefault [ gameparameternum ] = { 1,                       //       cgp_fahrspur                        
                                                        2,                       //       cgp_eis,                            
                                                        0,                       //       cgp_movefrominvalidfields,          
                                                        100,                     //       cgp_building_material_factor,       
                                                        100,                     //       cgp_building_fuel_factor,           
                                                        1,                       //       cgp_forbid_building_construction,   
                                                        0,                       //       cgp_forbid_unitunit_construction,   
                                                        0,                       //       cgp_bi3_training,                   
                                                        1,                       //       cgp_maxminesonfield,                
                                                        0,                       //       cgp_antipersonnelmine_lifetime,     
                                                        0,                       //       cgp_antitankmine_lifetime,          
                                                        0,                       //       cgp_mooredmine_lifetime,            
                                                        0,                       //       cgp_floatingmine_lifetime,          
                                                        100,                     //       cgp_buildingarmor,                  
                                                        100,                     //       cgp_maxbuildingrepair,              
                                                        100,                     //       cgp_buildingrepairfactor,           
                                                        1,                       //       cgp_globalfuel,                     
                                                        maxunitexperience,       //       cgp_maxtrainingexperience,          
                                                        0,                       //       cgp_initialMapVisibility,           
                                                        40,                      //       cgp_attackPower,                    
                                                        100,                     //       cgp_jammingAmplifier,               
                                                        10,                      //       cgp_jammingSlope,                   
                                                        0,                       //       cgp_superVisorCanSaveMap,           
                                                        1,                       //       cgp_objectsDestroyedByTerrain,      
                                                        2,                       //       cgp_trainingIncrement,              
                                                        1,                       //       gp_experienceDivisorAttack
                                                        0,                       //       cgp_disableDirectView
                                                        0,                       //       cgp_disableUnitTransfer
                                                        1,                       //       cgp_experienceDivisorDefense
                                                        0,                       //       cgp_debugEvents
                                                        0,                       //       cgp_objectGrowthMultiplier
                                                        0 };                     //       cgp_objectGrowOnOtherObjects


const bool gameParameterChangeableByEvent [ gameparameternum ] = { true,   //       cgp_fahrspur
                                                                 true,     //       cgp_eis,
                                                                 true,     //       cgp_movefrominvalidfields,
                                                                 true,     //       cgp_building_material_factor,
                                                                 true,     //       cgp_building_fuel_factor,
                                                                 true,     //       cgp_forbid_building_construction,
                                                                 true,     //       cgp_forbid_unitunit_construction,
                                                                 true,     //       cgp_bi3_training,
                                                                 true,     //       cgp_maxminesonfield,
                                                                 true,     //       cgp_antipersonnelmine_lifetime,
                                                                 true,     //       cgp_antitankmine_lifetime,
                                                                 true,     //       cgp_mooredmine_lifetime,
                                                                 true,     //       cgp_floatingmine_lifetime,
                                                                 true,     //       cgp_buildingarmor,
                                                                 true,     //       cgp_maxbuildingrepair,
                                                                 true,     //       cgp_buildingrepairfactor,
                                                                 true,     //       cgp_globalfuel,
                                                                 true,     //       cgp_maxtrainingexperience,
                                                                 true,     //       cgp_initialMapVisibility,
                                                                 true,     //       cgp_attackPower,
                                                                 true,     //       cgp_jammingAmplifier,
                                                                 true,     //       cgp_jammingSlope,
                                                                 false,    //       cgp_superVisorCanSaveMap,
                                                                 true,     //       cgp_objectsDestroyedByTerrain,
                                                                 true,     //       cgp_trainingIncrement,
                                                                 false,    //       cgp_experienceDivisorAttack };
                                                                 false,    //       cgp_disableDirectView
                                                                 false,    //       cgp_disableUnitTransfer
                                                                 false,    //       cgp_experienceDivisorDefense
                                                                 true,     //       cgp_debugEvents
                                                                 true,     //       cgp_objectGrowthMultiplier
                                                                 false };  //       cgp_objectGrowOnOtherObjects

const int gameParameterLowerLimit [ gameparameternum ] = { 1,    //       cgp_fahrspur
                                                           1,    //       cgp_eis,
                                                           0,    //       cgp_movefrominvalidfields,
                                                           1,    //       cgp_building_material_factor,
                                                           1,    //       cgp_building_fuel_factor,
                                                           0,    //       cgp_forbid_building_construction,
                                                           0,    //       cgp_forbid_unitunit_construction,
                                                           0,    //       cgp_bi3_training,
                                                           0,    //       cgp_maxminesonfield,
                                                           0,    //       cgp_antipersonnelmine_lifetime,
                                                           0,    //       cgp_antitankmine_lifetime,
                                                           0,    //       cgp_mooredmine_lifetime,
                                                           0,    //       cgp_floatingmine_lifetime,
                                                           1,    //       cgp_buildingarmor,
                                                           0,    //       cgp_maxbuildingrepair,
                                                           1,    //       cgp_buildingrepairfactor,
                                                           0,    //       cgp_globalfuel,
                                                           0,    //       cgp_maxtrainingexperience,
                                                           0,    //       cgp_initialMapVisibility,
                                                           1,    //       cgp_attackPower,
                                                           0,    //       cgp_jammingAmplifier,
                                                           0,    //       cgp_jammingSlope,
                                                           0,    //       cgp_superVisorCanSaveMap,
                                                           0,    //       cgp_objectsDestroyedByTerrain,
                                                           1,    //       cgp_trainingIncrement,
                                                           1,    //       gp_experienceDivisorAttack
                                                           0,    //       cgp_disableDirectView
                                                           0,    //       cgp_disableUnitTransfer
                                                           1,    //       cgp_experienceDivisorDefense
                                                           0,    //       cgp_debugEvents
                                                           0,    //       cgp_objectGrowthMultiplier
                                                           0 };  //       cgp_objectGrowOnOtherObjects



const int gameParameterUpperLimit [ gameparameternum ] = { maxint,                //       cgp_fahrspur
                                                           maxint,                //       cgp_eis,
                                                           1,                     //       cgp_movefrominvalidfields,
                                                           maxint,                //       cgp_building_material_factor,
                                                           maxint,                //       cgp_building_fuel_factor,
                                                           1,                     //       cgp_forbid_building_construction,
                                                           2,                     //       cgp_forbid_unitunit_construction,
                                                           maxunitexperience,     //       cgp_bi3_training,
                                                           maxint,                //       cgp_maxminesonfield,
                                                           maxint,                //       cgp_antipersonnelmine_lifetime,
                                                           maxint,                //       cgp_antitankmine_lifetime,
                                                           maxint,                //       cgp_mooredmine_lifetime,
                                                           maxint,                //       cgp_floatingmine_lifetime,
                                                           maxint,                //       cgp_buildingarmor,
                                                           100,                   //       cgp_maxbuildingrepair,
                                                           maxint,                //       cgp_buildingrepairfactor,
                                                           1,                     //       cgp_globalfuel,
                                                           maxunitexperience,     //       cgp_maxtrainingexperience,
                                                           2,                     //       cgp_initialMapVisibility,
                                                           100,                   //       cgp_attackPower,
                                                           1000,                  //       cgp_jammingAmplifier,
                                                           100,                   //       cgp_jammingSlope,
                                                           1,                     //       cgp_superVisorCanSaveMap,
                                                           1,                     //       cgp_objectsDestroyedByTerrain,
                                                           maxunitexperience,     //       cgp_trainingIncrement,
                                                           10,                    //       cgp_experienceDivisorAttack;
                                                           1,                     //       cgp_disableDirectView
                                                           1,                     //       cgp_disableUnitTransfer
                                                           10,                    //       cgp_experienceDivisorDefense
                                                           2,                     //       cgp_debugEvents
                                                           maxint,                //       cgp_objectGrowthMultiplier
                                                           1 };                   //       cgp_objectGrowOnOtherObjects


const char* gameparametername[ gameparameternum ] = { "lifetime of tracks",
                                                      "freezing time of icebreaker fairway",
                                                      "move vehicles from unaccessible fields",
                                                      "building construction material factor (percent)",
                                                      "building construction fuel factor (percent)",
                                                      "forbid construction of buildings",
                                                      "limit construction of units by other units",
                                                      "use BI3 style training factor ",
                                                      "maximum number of mines on a single field",
                                                      "lifetime of antipersonnel mine",
                                                      "lifetime of antitank mine",
                                                      "lifetime of antisub mine",
                                                      "lifetime of antiship mine",
                                                      "building armor factor (percent)",
                                                      "max building damage repair / turn",
                                                      "building repair cost increase (percent)",
                                                      "fuel globally available (BI Resource Mode)",
                                                      "maximum experience that can be gained by training",
                                                      "initial map visibility",
                                                      "attack power (EXPERIMENTAL!)",
                                                      "jamming amplifier (EXPERIMENTAL!)",
                                                      "jamming slope (EXPERIMENTAL!)",
                                                      "The Supervisor may save a game as new map (spying!!!)",
                                                      "objects can be destroyed by terrain",
                                                      "training centers: training increment",
                                                      "experience effect divisor for attack",
                                                      "disable direct View",
                                                      "disable transfering units/buildings to other players",
                                                      "experience effect divisor for defense",
                                                      "debug game events",
                                                      "Object growth rate (percentage)",
                                                      "Objects can grow on files with other objects" };


