/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "../actions/moveunitcommand.h"
#include "../loaders.h"
#include "../viewcalculation.h"
#include "../turncontrol.h"

#include "unittestutil.h"

class NextTurnStrategy_Abort : public NextTurnStrategy {
   public:
      bool continueWhenLastPlayer() const { 
         return false;
      };  
} ;


GameMap* startMap( const ASCString& filename )
{
   GameMap* game = mapLoadingExceptionChecker( filename, MapLoadingFunction( tmaploaders::loadmap  ));
   if ( !game )
      throw TestFailure("could not load map " + filename);;

   game->levelfinished = false;

   if ( game->replayinfo ) {
      delete game->replayinfo;
      game->replayinfo = NULL;
   }
   
   computeview( game );
         
   if ( game && game->actplayer == -1 ) 
      next_turn(game, NextTurnStrategy_Abort(), -1);
   
   return game;
   
}

Context createTestingContext( GameMap* gamemap )
{
   Context context;
   
   context.gamemap = gamemap;
   context.actingPlayer = &gamemap->getPlayer( gamemap->actplayer );
   context.parentAction = NULL;
   context.display = NULL;
   context.viewingPlayer = gamemap->getPlayerView(); 
   context.actionContainer = &gamemap->actions;
   return context;   
}
