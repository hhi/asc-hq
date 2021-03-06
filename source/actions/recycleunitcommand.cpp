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


#include "recycleunitcommand.h"

#include "../vehicle.h"
#include "../mapfield.h"
#include "../gamemap.h"
#include "../viewcalculation.h"
#include "../spfst.h"
#include "../mapdisplayinterface.h"
#include "action-registry.h"
#include "../itemrepository.h"
#include "../containercontrols.h"
#include "consumeresource.h"
#include "servicecommand.h"
#include "destructcontainer.h"

bool RecycleUnitCommand :: avail ( const ContainerBase* carrier, const Vehicle* unit )
{
   if ( !carrier || !unit )
      return false;
   
   if ( !carrier->findUnit( unit->networkid, false ) )
      return false;
   
   if ( !carrier->isBuilding() )
      return false;
        
   return true;
}


Resources RecycleUnitCommand::getOutput( const ContainerBase* carrier, const Vehicle* unit )
{
   if ( !avail( carrier, unit ))
      return Resources();
   
   int   output;
   if ( carrier->baseType->hasFunction( ContainerBaseType::RecycleUnits ) )
      output = recyclingoutput;
   else
      output = destructoutput;
   
   Resources res;

   for ( ContainerBase::Cargo::const_iterator i = unit->getCargo().begin(); i != unit->getCargo().end(); i++)
      if ( *i )
         res += getOutput ( carrier, *i );
   
   res.material += unit->typ->productionCost.material * (100 - unit->damage/2 ) * output / (100*100);
   return res;
   
}


RecycleUnitCommand :: RecycleUnitCommand ( ContainerBase* container )
   : ContainerCommand ( container ), unitID(-1)
{

}



 

ActionResult RecycleUnitCommand::go ( const Context& context )
{
   if ( getState() != SetUp )
      return ActionResult(22000);

   Vehicle* unit = getMap()->getUnit( unitID );
   if ( !unit )
      return ActionResult( 22100 );
   
   if ( !avail( getContainer(), unit ))
      return ActionResult( 22101 );
   
   auto_ptr<ServiceCommand> service ( new ServiceCommand( getContainer()));
   
   const ServiceTargetSearcher::Targets& targets = service->getDestinations();
   if ( find( targets.begin(), targets.end(), unit ) != targets.end() ) {
      service->setDestination( unit );  
      
      service->getTransferHandler( ).emptyDest();
      service->saveTransfers();
      ActionResult res = service->execute( context );
      if ( res.successful() )
         service.release();
      else
         return res;
   }
   
   Resources resource = getOutput ( getContainer(), unit );

   resource = getContainer()->putResource( resource, true, 1, context.actingPlayer->getPosition() );
   
   ActionResult res = (new ConsumeResource(getContainer(), -resource))->execute(context);
   
   if ( res.successful() )
      res = (new DestructContainer( unit))->execute( context );
   
   if ( res.successful() )
      setState( Finished );
   else
      setState( Failed );
   
   return res;
}



static const int RecycleUnitCommandVersion = 1;

void RecycleUnitCommand :: readData ( tnstream& stream )
{
   ContainerCommand::readData( stream );
   int version = stream.readInt();
   if ( version > RecycleUnitCommandVersion )
      throw tinvalidversion ( "RecycleUnitCommand", RecycleUnitCommandVersion, version );
   unitID = stream.readInt();
}

void RecycleUnitCommand :: writeData ( tnstream& stream ) const
{
   ContainerCommand::writeData( stream );
   stream.writeInt( RecycleUnitCommandVersion );
   stream.writeInt( unitID );
}

void RecycleUnitCommand :: setUnit( Vehicle* unit )
{
   unitID = unit->networkid;
   setState( SetUp );
}


ASCString RecycleUnitCommand :: getCommandString() const
{
   ASCString c;
   c.format("recycleUnit ( map, %d, %d )", getContainerID(), unitID );
   return c;

}

GameActionID RecycleUnitCommand::getID() const
{
   return ActionRegistry::RecycleUnitCommand;
}

ASCString RecycleUnitCommand::getDescription() const
{
   ASCString s = "Recycle ";
   
   if ( getMap()->getUnit( unitID ))
      s += getMap()->getUnit( unitID )->getName();
   
   if ( getContainer(true) ) {
      s += " by " + getContainer()->getName();
   }
   return s;
}

namespace
{
   const bool r1 = registerAction<RecycleUnitCommand> ( ActionRegistry::RecycleUnitCommand );
}

