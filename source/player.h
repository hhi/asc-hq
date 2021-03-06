/***************************************************************************
                          gamemap.h  -  description
                             -------------------
    begin                : Tue Feb 17 2001
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


#ifndef playerH
 #define playerH

 #include <vector>

 #include "typen.h"
 #include "vehicle.h"
 #include "buildings.h"
 #include "basestrm.h"
 #include "research.h"
 #include "password.h"
 #include "messages.h"
 #include "networkinterface.h"
 #include "playercolor.h"
 

class Player;


//! convenience-class which automatically determines the Player of units, buildings and other game objects
class PlayerID {
      int num;
   public:
      PlayerID( int num ) { this->num = num; };
      PlayerID( const ContainerBase* c ) : num( c->getOwner() ) {};
      PlayerID( const ContainerBase& c ) : num( c.getOwner() ) {};
      PlayerID( const Player& p );
      PlayerID( const PlayerID& p ) : num( p.getID() ) {};
      int getID() const { return num; };
};


const int diplomaticStateNum = 5;
enum DiplomaticStates { WAR, TRUCE, PEACE, PEACE_SV, ALLIANCE };
extern const char* diplomaticStateNames[diplomaticStateNum+1];


class DiplomaticStateVector : public sigc::trackable {

      friend class AllianceSetupWidget;
      friend class ChangeDiplomaticState;
      friend class DiplomacyCommand;

      Player& player; 
      
      typedef vector<DiplomaticStates> States;
      States states;
      
       
      typedef map<int,DiplomaticStates> QueuedStateChanges;
      QueuedStateChanges queuedStateChanges;

      void resize( int size );
      
   public:
      DiplomaticStateVector( Player& _player );
      
      DiplomaticStates getState( PlayerID towardsPlayer ) const;
      void setState( PlayerID towardsPlayer, DiplomaticStates s );
      
      bool isHostile( PlayerID towardsPlayer ) const { return getState( towardsPlayer.getID() ) == WAR; };
      bool sharesView( PlayerID receivingPlayer ) const { return getState( receivingPlayer.getID() ) >= PEACE_SV; };
      bool isAllied( PlayerID towardsPlayer ) const { return getState( towardsPlayer.getID() ) >= ALLIANCE; };

      void turnBegins();
      
      void swap( int secondPlayer );
            
      //! \returns true if there are any proposals
      bool getProposal( int fromPlayer, DiplomaticStates* state );
      
      void read ( tnstream& stream );
      void write ( tnstream& stream ) const;
};

class GameMap;


//! the different players in ASC. There may be 8 players (0..7) and neutral units (8)
class Player : public sigc::trackable {
      int player;
      GameMap* parentMap;
      
      void turnBegins( Player& p );
      void userInteractionBegins( Player& p );
      void turnEnds( Player& p );
      
      int serverPlayerID;

   public:
      Player();

      int getPosition() const { return player; };
      
      const GameMap* getParentMap() const { return parentMap; };
      GameMap* getParentMap() { return parentMap; };
      
      void setParentMap( GameMap* map, int pos );
      

      //! swaps all information, units, buildings etc with the given player
      void swap ( Player& secondPlayer );

      //! merges all units, buildings etc from secondplayer to this one
      void merge( Player& secondPlayer );


      //! does the player exist at all
      bool         exist() const;

      //! did the player exist when the turn started? Required for checking if a player has been terminated
      bool existanceAtBeginOfTurn;

      typedef list<Vehicle*> VehicleList;
      //! a list of all units
      VehicleList  vehicleList;

      typedef list<Building*> BuildingList;
      //! a list of all units
      BuildingList  buildingList;

      //! the status of the scientific research
      Research    research;
      
      Research& getResearch() { return research; };

      //! if the player is run by an AI, this is the pointer to it
      BaseAI*      ai;

      //! the status of the player
      enum PlayerStatus { human, computer, off, supervisor, suspended } stat;
      
      bool isHuman() const { return stat == human || stat == supervisor || stat == suspended; };

      static int getHumanPlayerNum( const GameMap* gamemap );

      static const char* playerStatusNames[];

      //! returns the name of the player 
      ASCString getName( ) const;
      void setName( const ASCString& name ) { this->name = name; };

      //! the Password required for playing this player
      Password passwordcrc;

      class Dissection {
         public:
            VehicleType*  fzt;
            const Technology*   tech;
            int           orgpoints;
            int           points;
            int           num;
      };

      //! the list of dissected units
      typedef list<Dissection> DissectionContainer;
      DissectionContainer dissections;

      bool __dissectionsToLoad;

      //! the list of messages that haven't been read by the player yet
      MessagePntrContainer  unreadmessage;
      bool __loadunreadmessage;

      //! the list of messages that already have been read by the player yet
      MessagePntrContainer  oldmessage;
      bool __loadoldmessage;

      //! the list of messages that have been sent yet
      MessagePntrContainer  sentmessage;
      bool __loadsentmessage;

      //! if ASC should check all events for fullfilled triggers, this variable will be set to true. This does not mean that there really ARE events that are ready to be executed
      int queuedEvents;

      //! the version of ASC that this player has used to make his last turn 
      int ASCversion;

      struct PlayTime {
         int turn;
         time_t date;
      };
      typedef list<PlayTime> PlayTimeContainer;

      //! The time this player ended his turns. This is very informative in email games with > 2 players to find out who is delaying the game.
      PlayTimeContainer playTime;

      MapCoordinate cursorPos;

      DiplomaticStateVector diplomacy;
      
      ASCString email;
                  
      DI_Color getColor() const;
      PlayerColor getPlayerColor() const;
      void setColor( const DI_Color& color ); 

      void read ( tnstream& stream );
      void write ( tnstream& stream ) const;

      void resetView();
      void resetResearch();
      void resetTribute();
      void resetPassword();

      bool operator==( const Player& otherPlayer) const;
      
      void setPlayerID( int id ) { serverPlayerID = id; };
      int getPlayerID() const { return serverPlayerID; };
      
   private:   
      ASCString       name;
      DI_Color        color;
      void sendQueuedMessages();
};



#endif

