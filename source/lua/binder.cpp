

extern "C"
{
    #include "lua.h"
}

#include <luabind/luabind.hpp>
#include <luabind/function.hpp>
#include <luabind/class.hpp>


#include "binder.h"
#include "luastate.h"

#include "../sg.h"
#include "../ascstring.h"
#include "../vehicle.h"
#include "../gamemap.h"
#include "../spfst.h"
               
#include "../actions/attackcommand.h"
#include "../actions/moveunitcommand.h"

using namespace luabind;

void loadGameLua( const std::string& filename )
{
   loadGameFromFile( ASCString(filename) );  
   repaintMap();
}
         
         
int attackCommandFunc( int veh, int x, int y )
{
   Vehicle* unit = actmap->getUnit( veh );
   if ( !unit  )
      return 120;
   
   if ( !AttackCommand::avail( unit ))   
      return 202;
   
   auto_ptr<AttackCommand> ac( new AttackCommand(unit) );
   ac->setTarget( MapCoordinate(x,y),-1);
   ActionResult res = ac->execute( createContext( actmap ) );
   if ( res.successful() ) {
      ac.release();
      return 0;
   } else
      return res.getCode();
   
}
         
int moveCommandFunc( int veh, int x, int y, int height )
{
   Vehicle* unit = actmap->getUnit( veh );
   if ( !unit )
      return 120;
   
   if ( !MoveUnitCommand::avail( unit ))   
      return 202;
   
   auto_ptr<MoveUnitCommand> ac( new MoveUnitCommand(unit) );
   MapCoordinate3D destination;
   destination.setnum( x,y,height );
   ac->setDestination( destination );
   ActionResult res = ac->execute( createContext( actmap ) );
   if ( res.successful() ) {
      ac.release();
      return 0;
   } else
      return res.getCode();
   
}
         
         
void registerLuaFunctions( LuaState& state)
{

    module( state.get() )
    [
        def("loadgame", &loadGameLua )
    ]; 
   
    module( state.get() )
    [
        def("attack", &attackCommandFunc )
    ]; 
    
    module( state.get() )
    [
        def("moveunit", &moveCommandFunc )
    ]; 
    
}
