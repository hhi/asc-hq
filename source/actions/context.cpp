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

#include <string>
#include "context.h"

Context::Context()
   : gamemap(NULL),
   actingPlayer(NULL),
   parentAction(NULL),
   display(NULL),
   viewingPlayer(-1),
   actionContainer(NULL)
{
}

Context::Context( const Context& context, GameAction* action )
{
   gamemap = context.gamemap;
   actingPlayer = context.actingPlayer;
   parentAction = action;
   display = context.display;
   viewingPlayer = context.viewingPlayer;
   actionContainer = NULL;
}
