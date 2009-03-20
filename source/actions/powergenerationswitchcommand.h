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


#ifndef PowerGenerationSwitchCommandH
#define PowerGenerationSwitchCommandH

#include "unitcommand.h"

#include "../typen.h"
#include "../objects.h"
#include "../mapfield.h"


class PowerGenerationSwitchCommand : public UnitCommand {
      bool newState;
   public:
      static bool avail ( const Vehicle* unit, bool newState );
   private:
      
      PowerGenerationSwitchCommand( GameMap* map ) : UnitCommand( map ), newState( false ) {};
      template<class Child> friend GameAction* GameActionCreator( GameMap* map);
      
   protected:
      void readData ( tnstream& stream );
      void writeData ( tnstream& stream ) const;
      
      GameActionID getID() const;
      ASCString getDescription() const;
      
   public:
      PowerGenerationSwitchCommand ( Vehicle* carrier );
      
      ActionResult go ( const Context& context ); 
      ASCString getCommandString() const;
      
      void setNewState( bool enabled );
};

#endif

