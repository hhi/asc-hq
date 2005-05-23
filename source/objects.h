/*! \file gamemap.h
    \brief Definition of THE central asc class: tmap 
*/

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


#ifndef objectsH
 #define objectsH

 #include <vector>

 #include "typen.h"
 #include "vehicle.h"
 #include "buildings.h"
 #include "basestrm.h"
 #include "research.h"

class AgeableItem {
    protected:
       AgeableItem() : lifetimer(-1) {};
    public:
       int lifetimer;

       //! ages the object by one turn. Returns true if the object shall be removed
       static bool age( AgeableItem& obj );
};

//! an instance of an object type (#tobjecttype) on the map
class Object : public AgeableItem {
    public:
       pobjecttype typ;
       int damage;
       int dir;
       // int dummy[4];
       Object ( void );
       Object ( pobjecttype t );
       void display ( Surface& surface, SPoint pos, int weather = 0 ) const;
       const OverviewMapImage* getOverviewMapImage( int weather );
       void setdir ( int dir );
       int  getdir ( void );

};

#define cminenum 4
extern const char* MineNames[cminenum] ;
extern const int MineBasePunch[cminenum]  ;

enum MineTypes { cmantipersonnelmine = 1 , cmantitankmine, cmmooredmine, cmfloatmine  };


class Mine : public AgeableItem {
   public:
      Mine( MineTypes type, int strength, int player, tmap* gamemap );


      MineTypes type;

      //! the effective punch of the mine
      int strength;

      //! the player who placed the mine; range 0 .. 7
      int player;

      //! can the mine attack this unit
      bool attacksunit ( const Vehicle* veh );
      
      //! display the mine
      void paint( Surface& surf, SPoint pos );

      //! display the mine
      static void paint( MineTypes type, int player, Surface& surf, SPoint pos );
          
};



#endif

