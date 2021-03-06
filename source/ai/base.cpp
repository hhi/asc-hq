/***************************************************************************
                          base.cpp  -  description
                             -------------------
    begin                : Fri Mar 30 2001
    copyright            : (C) 2001 by Martin Bickel
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

#include "ai_common.h"

#include "../replaymapdisplay.h"
#include "../turncontrol.h"
#include "../widgets/textrenderer.h"
#include "../mapdisplay.h"
#include "../asc-mainscreen.h"
#include "../gameeventsystem.h"
#include "../actions/diplomacycommand.h"

AI :: AI ( GameMap* _map, int _player ) : activemap ( _map ) , sections ( this )
{
   strictChecks = false;
   benchMark = false;

   player = _player;

   isRunning(false);
   fieldInformation = NULL;

   reset();
}

void AI :: reset ( void )
{
   maxTrooperMove = 0;
   maxTransportMove = 0;
   maxUnitMove = 0;

   for ( int i= 0; i < 8; i++ )
      maxWeapDist[i] = -1;
   baseThreatsCalculated = 0;

   if ( fieldInformation )
      delete[] fieldInformation;

   fieldInformation = NULL;
   fieldNum = 0;

   config.wholeMapVisible = 1;
   config.lookIntoTransports = 1;
   config.lookIntoBuildings = 1;
   config.aggressiveness  = 3;
   config.damageLimit = 70;
   config.resourceLimit = Resources ( 0, 5, 20 );
   config.ammoLimit= 10;
   config.maxCaptureTime = 15;
   config.maxTactTime = 10*100;
   config.waitForResourcePlus = 2;

   sections.reset();
}



void    AI :: setup (void)
{
   unitsWorkedInTactics.clear();
   
   displaymessage2("calculating all threats ... ");
   calculateAllThreats ();

   displaymessage2("calculating field threats ... ");
   calculateFieldInformation();

   displaymessage2("calculating sections ... ");
   sections.calculate();

/*
   for ( i = 0; i <= 8; i++) {
      Building* building = actmap->player[i].firstbuilding;
      while (building != NULL) {
         generatethreatvaluebuilding(building);
         building = building->next;
      }
      if (i == actmap->actplayer) {
         building = actmap->player[i].firstbuilding;
         while (building != NULL) {
            tcmpcheckreconquerbuilding ccrcb;
            ccrcb.init(3);
            ccrcb.initsuche(building->xpos,building->ypos,(maxfusstruppenmove + maxtransportmove) / 8 + 1,0);
            ccrcb.startsuche();
            int j;
            ccrcb.returnresult( &j );
            ccrcb.done();
            building = building->next;
         }
      }
   }
   */

   /*
    punits units = new tunits;
    tjugdesituationspfd jugdesituationspfd;
    jugdesituationspfd.init(units,1);
    jugdesituationspfd.startsuche();
    jugdesituationspfd.done();
    delete units;
   */

   for ( int p = 0; p < 8; p++ )
      getMap()->player[p].existanceAtBeginOfTurn = getMap()->player[p].exist();


   for ( Player::VehicleList::iterator i = getPlayer().vehicleList.begin(); i != getPlayer().vehicleList.end(); ++i)
      if ( (*i)->typ->hasFunction( ContainerBaseType::MoveWithReactionFire ) )
          if ( (*i)->reactionfire.getStatus() == Vehicle::ReactionFire::off )
             (*i)->reactionfire.enable();

/*
   // this is a cheat to speed up movement and improve Ai behaviour
   // setting all fields to FogOfWar
   for ( int y = 0; y < getMap()->ysize; ++y )
      for ( int x = 0; x < getMap()->xsize; ++x ) {
         tfield* fld = getMap()->getField(x,y);
         if ( fieldVisibility(fld) == visible_not )
            fld->setVisibility( visible_ago, getPlayerNum( ));
      }
  */       
   


   // showthreats("init: threatvals generated");
   displaymessage2("setup completed ... ");
}


void AI::checkKeys ( void )
{
   ASC_PG_App* app = &getPGApplication();
   if ( app )
      for ( int i = 0; i < 5; ++i )
         app->processEvent();
}

void AI::removeDisplay()
{
   mapDisplay = NULL;
}


typedef Loki::Functor<void> CloseScreenCallback;


class AI_KeyboardWatcher : public sigc::trackable {
      CloseScreenCallback callback;
      MapDisplayPG::LockDisplay* lock;
      PG_Widget* w;
      MainScreenWidget::StandardActionLocker menuLocker;

      bool keyPressed( const PG_MessageObject* object , const SDL_KeyboardEvent* key )
      {
         if ( key->keysym.sym == SDLK_ESCAPE  ) {
            callback();

            if ( !lock )
               lock = new MapDisplayPG::LockDisplay;

            return true;
         } else
            return false;
      }

   public:   
      AI_KeyboardWatcher( CloseScreenCallback callback ) : lock(NULL), w(NULL), menuLocker( mainScreenWidget, MainScreenWidget::LockOptions::Menu + MainScreenWidget::LockOptions::MapActions + MainScreenWidget::LockOptions::MapControl)
      {
         
         // w = new PG_Widget(NULL);
         // w->SetCapture();
         this->callback = callback;
         // w->sigKeyDown.connect( sigc::mem_fun( *this, &AI_KeyboardWatcher::keyPressed ));
         if ( PG_Application::GetApp() )
            PG_Application::GetApp()->sigKeyDown.connect( sigc::mem_fun( *this, &AI_KeyboardWatcher::keyPressed ));
         
      };

      void release()
      {
         delete w;
         w = NULL;
         delete lock;
         lock = NULL;
      }

      ~AI_KeyboardWatcher() 
      {
         release();
      }
};

void AI :: checkGameEvents()
{
   while ( getMap()->player[ getMap()->actplayer ].queuedEvents ) {
      if ( !checkevents( getMap(), mapDisplay ))
         return ;

      for ( Player::VehicleList::iterator vi = getPlayer().vehicleList.begin(); vi != getPlayer().vehicleList.end(); vi++ ) {
         Vehicle* veh = *vi;
         if ( !veh->aiparam[ getPlayerNum() ] )
            calculateThreat ( veh );
      }

   }

   checktimedevents( getMap(), mapDisplay );
}

class FieldMarkingSuppressor {
   MapDisplayInterface* mdi;
   public: 
      FieldMarkingSuppressor( MapDisplayInterface* mdi  ) {
         this->mdi = mdi;
         if ( mdi )
            mdi->setTempView( false );
      }
         
         ~FieldMarkingSuppressor() {
            if ( mdi )
               mdi->setTempView( true );
         }
};

void AI:: run ( bool benchMark, MapDisplayInterface* myMapDisplay )
{
   AI_KeyboardWatcher kw ( CloseScreenCallback( this, &AI::removeDisplay )); 

   this->benchMark = benchMark;

   auto_ptr<ReplayMapDisplay> rmd;
   if ( getMap()->getPlayerView() >= 0 && !benchMark && myMapDisplay ) {
      rmd.reset( new ReplayMapDisplay ( myMapDisplay ) );
      rmd->setCursorDelay (CGameOptions::Instance()->replayspeed + 30 );

      mapDisplay = rmd.get();
   } else 
      mapDisplay = NULL;

   int startTime = ticker;
   AiResult res;

   unitCounter = 0;
   isRunning(true);
   setVision(visible_ago);

   int setupTime = ticker;
   
   FieldMarkingSuppressor fms( mapDisplay );
   
   setup();

//   GameMap::Weather weatherBackup = getMap()->weather;
//   for ( int i = 0; i < 3; i++ )
//      getMap()->weather.wind[i].speed = 0;

   diplomacy();

   if ( !originalUnitDistribution.calculated )
      originalUnitDistribution = calcUnitDistribution();

   calcReconPositions();

   setupTime = ticker-setupTime;

   int serviceTime = ticker;
   issueServices( );
   executeServices();

   checkGameEvents();

   serviceTime = ticker-serviceTime;

   int conquerTime = ticker;
   checkConquer();
   conquerTime = ticker - conquerTime;

   runReconUnits ( );

   int containerTime = ticker;
   buildings( 3 );
   transports ( 3 );
   containerTime = ticker-containerTime;

   int tacticsTime = ticker;
   do {
      res = tactics();
      checkGameEvents();
   } while ( res.unitsMoved );
   tacticsTime = ticker - tacticsTime;

   int strategyTime = ticker;
   strategy();
   checkGameEvents();

   strategyTime = ticker - strategyTime;

   buildings( 1 );
   transports ( 3 );

   production();

   checkGameEvents();

   isRunning(false);
   if ( !mapDisplay )
      repaintMap();
   int duration = ticker-startTime;

/*
   if ( getMap()->replayinfo )
      getMap()->replayinfo->closeLogging();
*/
   
//   getMap()->weather = weatherBackup;

   if ( benchMark ) // (CGameOptions::Instance()->runAI == 2 && duration > 100*60) ||
      displaymessage ("The AI took %.2f seconds to run\n"
                      " setup: %d \n"
                      " service: %d \n"
                      " conquer: %d \n"
                      " container: %d \n"
                      " tactics: %d \n"
                      " strategy: %d \n",
        3, float(duration)/100,
           setupTime / 100,
           serviceTime/100,
           conquerTime /100,
           containerTime/100,
           tacticsTime/100,
           strategyTime/100 );

   displaymessage2("AI completed in %d second", duration/100);

   checkforvictory( getMap(), false );

   mapDisplay = NULL;
   
}


void AI :: diplomacy ()
{
   for ( int i = 0; i < getMap()->getPlayerCount(); ++i ) {
      if ( i != getPlayerNum() ) {
         DiplomaticStates proposal;
         if ( getPlayer().diplomacy.getProposal( i, &proposal )) {
            // the AI will not accept peace
            if ( proposal > getPlayer().diplomacy.getState( i ))
               new Message( "Your diplomatic proposal is declined", getMap(), 1 << i, 1 << getPlayerNum() );
            else {
               auto_ptr<DiplomacyCommand> dc ( new DiplomacyCommand( getPlayer()));  
               dc->newstate( proposal, getMap()->getPlayer(i) );
               ActionResult res = dc->execute( getContext() );
               if ( res.successful() )
                  dc.release();
            }
         }
      }
   }
}

void AI :: showFieldInformation ( int x, int y )
{
   if ( !fieldInformation )
      calculateFieldInformation();

   const char* fieldinfo = "#font02#Field Information (%d,%d)#font01##aeinzug20##eeinzug10##crtp10#"
                           "threat orbit: %d\n"
                           "threat high-level flight: %d\n"
                           "threat flight: %d\n"
                           "threat low-level flight: %d\n"
                           "threat ground level: %d\n"
                           "threat floating: %d\n"
                           "threat submerged: %d\n"
                           "threat deep submerged: %d\n"
                           "controlled by %d\n";

   char text[10000];
   AiThreat& threat = getFieldThreat ( x, y );
   sprintf(text, fieldinfo, x,y,threat.threat[7], threat.threat[6], threat.threat[5],
                                threat.threat[4], threat.threat[3], threat.threat[2],
                                threat.threat[1], threat.threat[0], getFieldInformation(x,y).control );

   MapField* fld = getMap()->getField (x, y );
   if ( fld->vehicle && fieldvisiblenow ( fld ) && fld->vehicle->aiparam[getPlayerNum()] ) {
      char text2[1000];
      sprintf(text2, "\nunit nwid: %d ; typeid: %d", fld->vehicle->networkid, fld->vehicle->typ->id );
      strcat ( text, text2 );
      AiParameter& aip = *fld->vehicle->aiparam[getPlayerNum()];

      if ( fld->vehicle->aiparam ) {

         sprintf(text2, "\nunit value: %d; xtogo: %d, ytogo: %d; ztogo: %d;\njob %s ; task %s \n", aip.getValue(), aip.dest.x, aip.dest.y, aip.dest.getBitmappedHeight(), AIjobs[aip.getJob()], AItasks[aip.getTask()] );
         strcat ( text, text2 );
      }

      if ( aip.dest.x >= 0 && aip.dest.y >= 0 ) {
         getMap()->cleartemps ( 1 );
         getMap()->getField ( aip.dest.x, aip.dest.y )->setaTemp(1);
      }


   }

   for ( ReconPositions::iterator i = reconPositions.begin(); i != reconPositions.end(); i++ )
      getMap()->getField( i->first )->setaTemp2(1);

   repaintMap();


   strcat ( text, "\n#font02#Section Information#font01##aeinzug20##eeinzug10##crtp10#");
   string s;

   Section& sec = sections.getForCoordinate ( x, y );

   s += "xp = ";
   s += strrr ( sec.xp );
   s += " ; yp = ";
   s += strrr ( sec.yp );
   s += "\n";
   const char* threattypes[4] = { "absUnitThreat", "avgUnitThreat", "absFieldThreat", "avgFieldThreat" };

   for ( int i = 0; i < 4; i++ ) {
      for ( int h = 0; h < 8; h++ ) {
         s += threattypes[i];
         s += " ";
         s += choehenstufen [h];
         s += " ";
         switch ( i ) {
            case 0: s += strrr ( sec.absUnitThreat.threat[h] );
                    break;
            case 1: s += strrr ( sec.avgUnitThreat.threat[h] );
                    break;
            case 2: s += strrr ( sec.absFieldThreat.threat[h] );
                    break;
            case 3: s += strrr ( sec.avgFieldThreat.threat[h] );
                    break;
         }
         s += "\n";
      }
      s += "\n";
   }

   for ( int j = 0; j < aiValueTypeNum; j++ ) {
      s+= "\nvalue ";
      s+= strrr ( j );
      s+= " = ";
      s+= strrr ( sec.value[j] );
   }

   strcat ( text, s.c_str() );

   ViewFormattedText vft( "AI information", text, PG_Rect( -1, -1, 500, 550 ) );
   vft.Show();
   vft.RunModal();
}


const int currentAiStreamVersion = 104;

void AI :: read ( tnstream& stream )
{
   int version = stream.readInt ( );
   if ( version > currentServiceOrderVersion )
      throw tinvalidversion ( "AI :: read", currentServiceOrderVersion, version );
   isRunning(stream.readInt ());
   setVision(VisibilityStates(stream.readInt ( )));
   unitCounter = stream.readInt ( );

   int i = stream.readInt();
   while ( i ) {
      ServiceOrder so ( this, stream );
      serviceOrders.push_back ( so );
      i = stream.readInt();
   }

   for_each ( serviceOrders.begin(), serviceOrders.end(), ServiceOrder::activate );

   i = stream.readInt();
   while ( i ) {
      MapCoordinate mc;
      mc.read ( stream );

      AI::BuildingCapture bc;
      bc.read ( stream );

      buildingCapture[mc] = bc;

      i = stream.readInt();
   }

   config.lookIntoTransports = stream.readInt();
   config.lookIntoBuildings = stream.readInt( );
   config.wholeMapVisible = stream.readInt( );
   config.aggressiveness = stream.readFloat( );
   config.damageLimit = stream.readInt();
   config.resourceLimit.read( stream );
   config.ammoLimit = stream.readInt();
   config.maxCaptureTime = stream.readInt();
   if ( version >= 102 )
      config.waitForResourcePlus = stream.readInt();

   if ( version >= 101 )
      config.maxTactTime = stream.readInt();

   if ( version >= 102 )
      originalUnitDistribution.read ( stream );

   if ( version >= 103 ) {
      int id = stream.readInt();
      while ( id >= 0 ) {
         float enemyValue = stream.readFloat();
         float ownValue = stream.readFloat();
         unitTypeSuccess[id] = make_pair ( enemyValue, ownValue );
         id = stream.readInt();
      }
   }

   if ( version >= 104 ) {
      int id = stream.readInt();
      while ( id >= 0 ) {
         stream.readFloat(); // enemyValue
         stream.readFloat(); // ownValue
         id = stream.readInt();
      }
   }

   int version2 = stream.readInt();
   if ( version != version2 )
      throw tinvalidversion ( "AI :: read", version, version2 );


}

void AI :: write ( tnstream& stream ) const
{
   const int version = currentAiStreamVersion;
   stream.writeInt ( version );
   stream.writeInt ( isRunning() );
   stream.writeInt ( getVision() );
   stream.writeInt ( unitCounter );

   for ( ServiceOrderContainer::const_iterator i = serviceOrders.begin(); i != serviceOrders.end(); i++) {
      stream.writeInt ( 1 );
      i->write ( stream );
   }

   stream.writeInt ( 0 );

   for ( map<MapCoordinate,BuildingCapture>::const_iterator i = buildingCapture.begin(); i != buildingCapture.end(); i++ ) {
      stream.writeInt ( 1 );
      i->first.write ( stream );
      i->second.write ( stream );
   }
   stream.writeInt ( 0 );

   stream.writeInt( config.lookIntoTransports );   /*  gegnerische transporter einsehen  */
   stream.writeInt( config.lookIntoBuildings );
   stream.writeInt( config.wholeMapVisible );
   stream.writeFloat( config.aggressiveness );   // 1: units are equally worth ; 2
   stream.writeInt( config.damageLimit );
   config.resourceLimit.write( stream );
   stream.writeInt( config.ammoLimit );
   stream.writeInt( config.maxCaptureTime );
   stream.writeInt( config.maxTactTime );
   stream.writeInt( config.waitForResourcePlus );

   originalUnitDistribution.write( stream );

   for ( UnitTypeSuccess::const_iterator i = unitTypeSuccess.begin(); i != unitTypeSuccess.end(); ++i ) {
     stream.writeInt( i->first );
     stream.writeFloat( i->second.first );
     stream.writeFloat( i->second.second );
   }
   stream.writeInt( -1 );
   stream.writeInt( -1 );

   stream.writeInt ( version );
}

Context AI :: getContext()
{
   Context context;
   
   context.gamemap = getMap();
   context.actingPlayer = &getPlayer();
   context.parentAction = NULL;
   context.display = mapDisplay;
   context.viewingPlayer = getMap()->getPlayerView(); 
   context.actionContainer = &getMap()->actions;
   return context;   
}

AI :: ~AI ( )
{
   if ( fieldInformation ) {
      delete[] fieldInformation;
      fieldInformation = NULL;
      fieldNum = 0;
   }
}


