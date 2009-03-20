/*
     This file is part of Advanced Strategic Command; http://www.asc-hq.de
     Copyright (C) 1994-2008  Martin Bickel  and  Marc Schellenberger
 
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


#ifndef SpawnBuildingH
#define SpawnBuildingH


#include "action.h"
#include "action-registry.h"

#include "../typen.h"
#include "../buildings.h"

class SpawnBuilding : public GameAction {
      MapCoordinate pos;
      int buildingTypeID;
      int owner;
      
      SpawnBuilding( GameMap* map ) : GameAction( map ) {};
      template<class Child> friend GameAction* GameActionCreator( GameMap* map);

   public:
      SpawnBuilding( GameMap* gamemap, const MapCoordinate& position, int buildingTypeID, int owner );
      
      ASCString getDescription() const;
      
      Building* getBuilding();
      
   protected:
      virtual GameActionID getID() const;
      
      virtual ActionResult runAction( const Context& context );
      virtual ActionResult undoAction( const Context& context );
      virtual ActionResult verify();
      
      virtual void readData ( tnstream& stream );
      virtual void writeData ( tnstream& stream ) const;
      
};

#endif
