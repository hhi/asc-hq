/*! \file attack.h
    \brief Interface for all the fighting routines of ASC. 
*/


//     $Id: attack.h,v 1.21 2003-05-01 18:02:22 mbickel Exp $
//
//     $Log: not supported by cvs2svn $
//     Revision 1.20  2001/10/21 13:16:59  mbickel
//      Cleanup and documentation
//
//     Revision 1.19  2001/02/26 12:35:00  mbickel
//      Some major restructuing:
//       new message containers
//       events don't store pointers to units any more
//       tfield class overhauled
//
//     Revision 1.18  2001/02/08 21:21:03  mbickel
//      AI attacks and services more sensibly
//
//     Revision 1.17  2001/01/28 14:04:02  mbickel
//      Some restructuring, documentation and cleanup
//      The resource network functions are now it their own files, the dashboard
//       as well
//      Updated the TODO list
//
//     Revision 1.16  2001/01/19 13:33:46  mbickel
//      The AI now uses hemming
//      Several bugfixes in Vehicle Actions
//      Moved all view calculation to viewcalculation.cpp
//      Mapeditor: improved keyboard support for item selection
//
//     Revision 1.15  2000/10/11 14:26:16  mbickel
//      Modernized the internal structure of ASC:
//       - vehicles and buildings now derived from a common base class
//       - new resource class
//       - reorganized exceptions (errors.h)
//      Split some files:
//        typen -> typen, vehicletype, buildingtype, basecontainer
//        controls -> controls, viewcalculation
//        spfst -> spfst, mapalgorithm
//      bzlib is now statically linked and sources integrated
//
//     Revision 1.14  2000/09/16 11:47:21  mbickel
//      Some cleanup and documentation again
//
//     Revision 1.13  2000/09/07 15:49:38  mbickel
//      some cleanup and documentation
//
//     Revision 1.12  2000/08/12 12:52:42  mbickel
//      Made DOS-Version compile and run again.
//
//     Revision 1.11  2000/08/05 13:38:20  mbickel
//      Rewrote height checking for moving units in and out of
//        transports / building
//
//     Revision 1.10  2000/08/03 13:11:50  mbickel
//      Fixed: on/off switching of generator vehicle produced endless amounts of energy
//      Repairing units now reduces their experience
//      negative attack- and defenseboni possible
//      changed attackformula
//
//     Revision 1.9  2000/07/16 14:19:59  mbickel
//      AI has now some primitive tactics implemented
//      Some clean up
//        moved weapon functions to attack.cpp
//      Mount doesn't modify PCX files any more.
//
//     Revision 1.8  2000/07/06 11:07:25  mbickel
//      More AI work
//      Started modularizing the attack formula
//
//     Revision 1.7  2000/06/08 21:03:39  mbickel
//      New vehicle action: attack
//      wrote documentation for vehicle actions
//
//     Revision 1.6  2000/06/04 21:39:18  mbickel
//      Added OK button to ViewText dialog (used in "About ASC", for example)
//      Invalid command line parameters are now reported
//      new text for attack result prediction
//      Added constructors to attack functions
//
//     Revision 1.5  2000/04/27 16:25:14  mbickel
//      Attack functions cleanup
//      New vehicle categories
//      Rewrote resource production in ASC resource mode
//      Improved mine system: several mines on a single field allowed
//      Added unitctrl.* : Interface for vehicle functions
//        currently movement and height change included
//      Changed timer to SDL_GetTicks
//
//     Revision 1.4  2000/01/24 08:16:49  steb
//     Changes to existing files to implement sound.  This is the first munge into
//     CVS.  It worked for me before the munge, but YMMV :)
//
//     Revision 1.3  2000/01/20 16:52:09  mbickel
//      Added Kamikaze attack
//
//     Revision 1.2  1999/11/16 03:41:04  tmwilson
//     	Added CVS keywords to most of the files.
//     	Started porting the code to Linux (ifdef'ing the DOS specific stuff)
//     	Wrote replacement routines for kbhit/getch for Linux
//     	Cleaned up parts of the code that gcc barfed on (char vs unsigned char)
//     	Added autoconf/automake capabilities
//     	Added files used by 'automake --gnu'
//
//
/*
    This file is part of Advanced Strategic Command; http://www.asc-hq.de
    Copyright (C) 1994-1999  Martin Bickel  and  Marc Schellenberger

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



#ifndef attack_h
#define attack_h

#include "typen.h"
#include "vehicletype.h"





class AttackFormula {
            virtual bool checkHemming ( pvehicle d_eht, int direc );
         public:
            float strength_experience ( int experience );
            float strength_damage ( int damage );
            float strength_attackbonus ( int abonus );
            float strength_hemming ( int  ax,  int ay,  pvehicle d_eht );
            float defense_experience ( int experience );
            float defense_defensebonus ( int defensebonus );
            static float getHemmingFactor ( int relDir );  // 0 <= reldir <= sidenum-2
        };

class tfight : public AttackFormula {
           void paintbar ( int num, int val, int col );
           void paintline ( int num, int val, int col );
           virtual void paintimages ( int xa, int ya, int xd, int yd ) = 0;
        protected:   
           tfight ( void );
           int synchronedisplay;
        public:  
           struct tavalues {
                     int strength;
                     int armor;
                     int damage;
                     int experience;
                     int defensebonus;
                     int attackbonus;
                     float hemming;
                     int weapnum;
                     int weapcount;
                     int color;
                     int initiative;
                     int kamikaze;
                  } av, dv;

           //! Performs the calculation of the attack. The result is only stored in the av and dv structures and is not written to the map
           void calc ( void ) ;
      
           //! Performs the calculation of the attack and displays the result on screen. As in calc , the result is not written to the actual units.
           virtual void calcdisplay ( int ad = -1, int dd = -1 );

           //! Writes the result of the attack calculation to the actual units.
           virtual void setresult ( void ) = 0;
      };

class tunitattacksunit : public tfight {
           pvehicle _attackingunit; 
           pvehicle _attackedunit; 

           pvehicle* _pattackingunit; 
           pvehicle* _pattackedunit; 

           int _respond; 
           void paintimages ( int xa, int ya, int xd, int yd );
         public:
           /*! Calculates the fight if one unit attacks another units.
               \param respond Does the unit that is being attacked retalliate ?
               \param weapon  The number of the weapon which the attacking unit attacks with. If it is -1, the best weapon is chosen.
           */
           tunitattacksunit ( pvehicle &attackingunit, pvehicle &attackedunit, bool respond = true, int weapon = -1);
           void setup ( pvehicle &attackingunit, pvehicle &attackedunit, bool respond, int weapon );
           void setresult ( void );
           virtual void calcdisplay(int ad = -1, int dd = -1);

      };

class tunitattacksbuilding : public tfight {
           pvehicle _attackingunit; 
           pbuilding _attackedbuilding; 
           int _x, _y;
           void paintimages ( int xa, int ya, int xd, int yd );
         public:
           /*! Calculates the fight if one unit attacks the building at coordinate x/y.
               \param weapon  The number of the weapon which the attacking unit attacks with. If it is -1, the best weapon is chosen.
           */
           tunitattacksbuilding ( pvehicle attackingunit, int x, int y, int weapon = -1);
           void setup ( pvehicle attackingunit, int x, int y, int weapon );
           void setresult ( void );
           virtual void calcdisplay(int ad = -1, int dd = -1);

      };


class tmineattacksunit : public tfight {
            pfield _mineposition;
            pvehicle _attackedunit;
            int _minenum;
            pvehicle* _pattackedunit;
            void paintimages ( int xa, int ya, int xd, int yd );
         public:
           /*! Calculates the fight if a unit drives onto a mine.
               \param mineposition The field on which the mine was placed  
               \param minenum The number of a specific mine which explodes. If -1 , all mines on this field which are able to attack the unit will explode.
               \param attackedunit The unit which moved onto the minefield.
           */
           tmineattacksunit ( pfield mineposition, int minenum, pvehicle &attackedunit );
           void setup ( pfield mineposition, int minenum, pvehicle &attackedunit );
           void setresult ( void );
           virtual void calcdisplay(int ad = -1, int dd = -1);
      };

class tunitattacksobject : public tfight {
           pvehicle     _attackingunit;
           pfield       targetField;
           pobject      _obji;
           void paintimages ( int xa, int ya, int xd, int yd );
           int _x, _y;
         public:
           /*! Calculates the fight if one unit attacks the objects at coordinate x/y.
               \param weapon  The number of the weapon which the attacking unit attacks with. If it is -1, the best weapon is chosen.
           */
           tunitattacksobject ( pvehicle attackingunit, int obj_x, int obj_y, int weapon = -1 );
           void setup ( pvehicle attackingunit, int obj_x, int obj_y, int weapon );
           void setresult ( void );
           virtual void calcdisplay(int ad = -1, int dd = -1);
      };

   //! Structure to store the weapons which a unit can use to perform an attack. \sa attackpossible  
   class AttackWeap {
               public:
                    int          count; 
                    int          strength[16]; 
                    int          num[16]; 
                    int          typ[16];

                    enum Target { nothing, vehicle, building, object } target;
                 }; 
   typedef class AttackWeap* pattackweap ;


//! \brief Is attacker able to attack anything in field x/y ?
extern pattackweap attackpossible( const pvehicle attacker, int x, int y);



/*! \brief Is attacker able to attack target ? Distance is not evaluated.

     The distance is not evaluated. The routine is used for the movement routines for example,
     because the current distance of units A and B is not relevant for the check whether unit 
     A can move across the field where B is standing.

     \param attackweap if != NULL, detailed information about the weapons which can perform
                          the attack are written to attackweap
*/
extern bool attackpossible2u( const pvehicle attacker, const pvehicle target, pattackweap attackweap = NULL, int uheight = -1);      // distance is not evaluated


/*! \brief Is attacker able to attack target ? Distance is assumed one field.

     The distance is assumed to be 1 field. The routine is used for the movement routines for 
     example, because units moving next to enemies get a movement malus.

     \param attackweap if != NULL, detailed information about the weapons which can perform
                          the attack are written to attackweap
*/
extern bool attackpossible28( const pvehicle attacker, const pvehicle target, pattackweap attackweap = NULL);       // distance is fixed as 1 field


/*! \brief Is attacker able to attack target ? Actual distance used.

     \param attackweap if != NULL, detailed information about the weapons which can perform
                          the attack are written to attackweap
*/
extern bool attackpossible2n( const pvehicle attacker, const pvehicle target, pattackweap attackweap = NULL );

//! Can the vehicle drive across the field and destroy any unit there by moving over them?
extern bool vehicleplattfahrbar( const pvehicle vehicle, const pfield field );


//! Some very old system to calculate the weapon efficiency over a given distance.
class WeapDist {
            char         data[7][256];        /* mg,bomb,gmissile,amissile,torpedo,cannon,cruise missile  */ 
         public:
            void loaddata ( void ) ;
            float getWeapStrength ( const SingleWeapon* weap, int dist =-1, int attacker_height =-1, int defender_height = -1, int reldiff = -1 );
         }; 

extern WeapDist weapDist;


#endif



