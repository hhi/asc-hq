/*
     This file is part of Advanced Strategic Command; http://www.asc-hq.de
     Copyright (C) 1994-2010  Martin Bickel  and  Marc Schellenberger
 
     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.
 
     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
 
     You should have received a copy of the GNU General Public License
     along with this program; see the file COPYING. If not, write to the 
     Free Software Foundation, Inc., 59 Temple Place, Suite 330, 
     Boston, MA  02111-1307  USA
*/


#ifndef MoveUnitH
#define MoveUnitH


#include "unitaction.h"
#include "../astar2.h"

class MoveUnit : public UnitAction {
      AStar3D::Path pathToMove;
      bool dontInterrupt;
      
      /* this is a workaround 
         The unit may have inconsistent combinations of height and movement during the operation
         If the unit is destroyed while it is in such a state, we need to restore the movement manually
      */
      int originalUnitMovement;
   
      
      MoveUnit( GameMap* map ) : UnitAction( map ) {};
      template<class Child> friend GameAction* GameActionCreator( GameMap* map);
      
   public:
      MoveUnit( Vehicle* veh, AStar3D::Path& pathToMove, bool dontInterrupt = false );
      
      ASCString getDescription() const;
      
   protected:
      virtual GameActionID getID() const;
      
      virtual ActionResult runAction( const Context& context );
      virtual ActionResult undoAction( const Context& context );
      virtual ActionResult preCheck();
      virtual ActionResult postCheck();
      
      virtual void readData ( tnstream& stream );
      virtual void writeData ( tnstream& stream ) const;
      
};

#endif

