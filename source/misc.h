//     $Id: misc.h,v 1.15 2000-12-21 16:41:56 mbickel Exp $
//
//     $Log: not supported by cvs2svn $
//     Revision 1.14  2000/10/14 10:52:52  mbickel
//      Some adjustments for a Win32 port
//
//     Revision 1.13  2000/10/12 22:24:00  mbickel
//      Made the DOS part of the new platform system work again
//
//     Revision 1.12  2000/10/12 21:37:53  mbickel
//      Further restructured platform dependant routines
//
//     Revision 1.11  2000/10/11 14:26:43  mbickel
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
//     Revision 1.10  2000/09/16 11:47:29  mbickel
//      Some cleanup and documentation again
//
//     Revision 1.9  2000/09/07 15:49:44  mbickel
//      some cleanup and documentation
//
//     Revision 1.8  2000/08/02 10:28:27  mbickel
//      Fixed: generator vehicle not working
//      Streams can now report their name
//      Field information shows units filename
//
//     Revision 1.7  2000/05/07 12:53:59  mbickel
//      some minor adjustments
//
//     Revision 1.6  2000/05/06 20:25:23  mbickel
//      Fixed: -recognition of a second mouse click when selection a pd menu item
//             -movement: fields the unit can only pass, but not stand on them,
//                        are marked darker
//             -intedit/stredit: mouseclick outside is like hitting enter
//
//     Revision 1.5  2000/03/11 18:22:07  mbickel
//      Added support for multiple graphic sets
//
//     Revision 1.4  2000/02/03 20:54:41  mbickel
//      Some cleanup
//      getfiletime now works under Linux too
//
//     Revision 1.3  1999/12/28 21:03:09  mbickel
//      Continued Linux port
//      Added KDevelop project files
//
//     Revision 1.2  1999/11/16 03:42:07  tmwilson
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

/** \file misc.h
    miscellaneous functions which are used by ASC and all its tools
*/


#ifndef misc_h
   #define misc_h
   
   #include "global.h"
   
   #define dblue lightgray
   //153
   
  #ifdef UseMemAvail 
   extern int memavail ( void );       // dummy function
  #endif

   extern int log2(int zahl);

   /** converts a to a string.
      \returns a pointer to a newly allocated array of characters, which must be freed with delete[]
    */
   extern char* strr ( int a ) ;
   
   extern const char* digit[] ; 
   extern const char* letter[] ;
   
   /** converts a to a string.
      \returns a pointer to a static array, which will be overwritten in the next call
    */
   extern char* strrr ( int a ) ;

   /** converts a to a string.
      \returns a pointer to a static array, which will be overwritten in the next call
    */
   extern char* strrr ( double a ) ;

   extern void beep( void );

   extern int  crc32buf( const void *vbuf, int len);

   extern int min ( int a, int b );
   extern int min ( int a, int b, int c );
   extern int max ( int a, int b );
   extern int max ( int a, int b, int c );

   extern char *strupr (const char *a);

  #ifndef HAVE_ITOA
   extern char* itoa ( int a, char* b, int c);
  #endif






 #pragma pack(1)



// is there any way to do this test at compile time ??
class structure_size_tester {       
        struct _test_structure {
            char a;
            char b;
            int c;
            char d;
            void* e[10];
        };
     public:
       structure_size_tester ( void );
    };

   #if defined(MEMCHK) | defined(sgmain) | defined(karteneditor)
    extern void* asc_malloc ( size_t size );
    extern void asc_free ( void* p );
   #else
    #define asc_malloc malloc
    #define asc_free   free
   #endif

   #pragma pack()
#endif
