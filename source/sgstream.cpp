//     $Id: sgstream.cpp,v 1.18 2000-07-31 18:02:54 mbickel Exp $
//
//     $Log: not supported by cvs2svn $
//     Revision 1.17  2000/07/29 15:28:36  mbickel
//      Plaintext configfile runs now in Linux version too
//
//     Revision 1.16  2000/07/29 14:54:43  mbickel
//      plain text configuration file implemented
//
//     Revision 1.15  2000/07/28 10:15:29  mbickel
//      Fixed broken movement
//      Fixed graphical artefacts when moving some airplanes
//
//     Revision 1.14  2000/06/28 19:26:17  mbickel
//      fixed bug in object generation by building removal
//      Added artint.cpp to makefiles
//      Some cleanup
//
//     Revision 1.13  2000/06/28 18:31:02  mbickel
//      Started working on AI
//      Started making loaders independent of memory layout
//      Destroyed buildings can now leave objects behind.
//
//     Revision 1.12  2000/05/23 20:40:49  mbickel
//      Removed boolean type
//
//     Revision 1.11  2000/05/06 20:25:24  mbickel
//      Fixed: -recognition of a second mouse click when selection a pd menu item
//             -movement: fields the unit can only pass, but not stand on them,
//                        are marked darker
//             -intedit/stredit: mouseclick outside is like hitting enter
//
//     Revision 1.10  2000/04/27 16:25:26  mbickel
//      Attack functions cleanup
//      New vehicle categories
//      Rewrote resource production in ASC resource mode
//      Improved mine system: several mines on a single field allowed
//      Added unitctrl.* : Interface for vehicle functions
//        currently movement and height change included
//      Changed timer to SDL_GetTicks
//
//     Revision 1.9  2000/03/29 09:58:48  mbickel
//      Improved memory handling for DOS version
//      Many small changes I can't remember ;-)
//
//     Revision 1.8  2000/03/11 18:22:08  mbickel
//      Added support for multiple graphic sets
//
//     Revision 1.7  2000/02/03 20:54:41  mbickel
//      Some cleanup
//      getfiletime now works under Linux too
//
//     Revision 1.6  2000/01/24 17:35:46  mbickel
//      Added dummy routines for sound under DOS
//      Cleaned up weapon specification
//
//     Revision 1.5  1999/12/28 21:03:20  mbickel
//      Continued Linux port
//      Added KDevelop project files
//
//     Revision 1.4  1999/12/27 13:00:10  mbickel
//      new vehicle function: each weapon can now be set to not attack certain
//                            vehicles
//
//     Revision 1.3  1999/11/22 18:27:54  mbickel
//      Restructured graphics engine:
//        VESA now only for DOS
//        BASEGFX should be platform independant
//        new interface for initialization
//      Rewrote all ASM code in C++, but it is still available for the Watcom
//        versions
//      Fixed bugs in RLE decompression, BI map importer and the view calculation
//
//     Revision 1.2  1999/11/16 03:42:28  tmwilson
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

                                               
#include "config.h"
#include <malloc.h>
#ifdef _DOS_
#include <dos.h> 
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream.h>
#include <math.h>
#include <stdarg.h>
#include <sys/types.h>
#include <dirent.h>


#include <sys/stat.h>


#include "CLoadable.h"

#include "tpascal.inc"
#include "typen.h"
#include "basegfx.h"
#include "misc.h"
#include "sgstream.h"
#include "stack.h"
#include "basestrm.h"
#include "palette.h"
#include "gameoptions.h"

#ifndef converter
 #include "keybp.h"
 #include "dlg_box.h"
 #include "dialog.h"
#endif

#ifdef HEXAGON
 #include "loadbi3.h"
#endif

#ifdef _DOS_
 #include "dos\\memory.h"
#endif


const char* asc_EnvironmentName = "asc_configfile";

#if defined(_DOS_) | defined(WIN32)
 const char* asc_configurationfile = "asc.ini";
#else
 const char* asc_configurationfile = "~/.asc/ascrc";
#endif


FILE* logfile = NULL;

#ifndef converter
void logtofile ( char* strng, ... )
{
   char buf[10000];
   va_list arglist;
   va_start ( arglist, strng );
   vsprintf ( buf, strng, arglist );

   // int a = memavail();

   if ( !logfile )
     logfile = fopen ( "SGLOG.TXT", "at+" );


  /*
   if ( crctest )
      fprintf ( f, "%d; ", crctest );
   */

#ifdef _DOS_
   if ( _heapchk() != _HEAPOK  )
     fprintf( logfile, "HEAP DAMAGED!!" );
#endif
   fprintf ( logfile, buf );
   fprintf ( logfile, "\n" );
   fflush ( logfile );
   va_end ( arglist );
}
#endif



#pragma pack(1)
struct tindexstruct {
                    char         filename[13]; 
                    char         archivename[31]; 
                    int      posstart, posstop; 
                 }; 


/*
void         tsrlefilestream::readrlepict( void** pnter, char allocated, int* size)
{
   readrlepict ( (char**) pnter, allocated. size );
}
*/

void         tsrlefilestream::readrlepict( char** pnter, char allocated, int* size)
{ 
  trleheader   hd; 
  word         w; 
  char*        q;

   if (status != 0) return;

   readdata( (char*)&hd, sizeof( hd ));

   if (hd.id == 16973) { 
      if (!allocated) 
        *pnter = (char*) asc_malloc ( hd.size + sizeof(hd) );
      memcpy( *pnter, &hd, sizeof(hd)); 
      q = *pnter + sizeof(hd);

      readdata( q, hd.size);
      *size = hd.size + sizeof(hd); 
   } 
   else { 
      w = (word) ( (hd.id + 1) * (hd.size + 1) + 4 ); 
      if (!allocated) 
        *pnter = (char*) asc_malloc ( w );
      memcpy ( *pnter, &hd, sizeof ( hd ));
      q = *pnter + sizeof(hd);
      readdata ( q, w - sizeof(hd) );
      *size = w; 
   } 
} 





void tsrlefilestream :: writerlepict ( void* buf )
{
   void* tempbuf = asc_malloc ( 0xffff );
   int   size    = compressrle ( buf, tempbuf );
   writedata ( (char*) tempbuf, size );
   asc_free ( tempbuf );
}



 /*  --------------- tstream -----------------  */ 



void tstream::writedata ( void* buf, int size ) {
   writedata ( (char*) buf, size );
}


void tstream::readdata  ( void* buf, int size ) {
   readdata ( (char*) buf, size );
}


void tstream::writedata ( char buf ) {
   writedata ( &buf, sizeof(char) );
}

void tstream::readdata  ( char* buf ) {
   readdata ( buf, 1 );
}


void tstream::writedata ( word buf ) {
   writedata ( (char*) &buf, sizeof( word ) );
}

void tstream::readdata  ( word* buf ) {
   readdata ( (char*) buf, sizeof ( word ) );
}


void tstream::writedata ( int buf ) {
   writedata ( (char*) &buf, sizeof ( int ) );
}

void tstream::readdata  ( int* buf ) {
   readdata ( (char*) buf, sizeof( int ) );
}
  /*  -------------- tbufstream -------------  */ 



char        tbufstream::getstatus(void)
{ 
   int s;
   s = status;
   status = 0;
   return (char) s;
} 


void         tbufstream::init(void)
{ 
   modus = 0; 
   int maxmemsize = 0xffff;

   devicename = "abstract"; 

  if (memavail() > maxmemsize)
      memsize = maxmemsize; 
   else 
      memsize = memavail(); 


   zeiger = (char*) asc_malloc( memsize );

   #ifdef dlg_box_h
   if (zeiger == NULL) 
      displaymessage("tbufstream::init \ncould not allocate memory !\nmemavail = %d \n",2,memavail());
   #endif

   datasize = 0; 

//   logfile = fopen ( "D:\\SG.LOG", "wt");
} 



void         tbufstream::readdata( char* buf, int size )
{ 
  void*      p; 
  int          s, actpos2; 

   if (status != 0) return;
   actpos2 = 0; 

   #ifdef dlg_box_h
   if (modus == 2) 
      displaymessage("attempt to read in a file opened only for reading\nfile: %s\n" ,2, devicename) ;
   #endif

   p = buf; 
   while (actpos2 < size) { 
      if (datasize == 0) { 

         #ifdef dlg_box_h
         displaymessage("data read error\nfile: %s\n",2,devicename) ;
         #endif
 
      } 
      s = datasize - actmempos; 
      if (s > size - actpos2) 
         s = size - actpos2; 

      memcpy ( buf + actpos2, zeiger + actmempos, s );

      actmempos = actmempos + s; 
      if (actmempos >= datasize) { 
         readbuffer();
         actmempos = 0; 
      } 
      actpos2 = actpos2 + s; 
   } 

} 


void         tbufstream::readpchar(char** pc)
{ 
  int          actpos2; 
  int          ms; 
  char         *pch1, *pch2; 

   if (status != 0) return;
   actpos2 = 0; 

   #ifdef dlg_box_h
   if (modus == 2) 
      displaymessage("attempt to read pchar from a file opened only for writing\nfile: %s\n", 2,devicename);
   #endif

   if (memavail() < 0xffff)
      ms = memavail(); 
   else  
      ms = 0xffff; 

   pch1 = (char*) asc_malloc ( 0xffff );

   pch2 = pch1;
   pch2--;

   do {
     actpos2++;
     pch2++;

     readdata( pch2, 1 );

   } while (*pch2 != 0  ); /* enddo */

   if ( actpos2 > 1 ) {
      pch2 = (char*) asc_malloc ( actpos2 );

      memcpy ( pch2, pch1, actpos2 );
      *pc = pch2;

   } else
      *pc = NULL;
   asc_free( pch1 );

} 

void         tbufstream::writepchar(char* pc)
{ 
  if ( pc ) {
     char         *pch1 = pc;
   
     pch1--;
     do {
        pch1++;
        writedata( pch1, 1 );
     } while ( *pch1 > 0  ); /* enddo */
  } else {
     char  a = 0;
     char* b = &a;
     writedata( b, 1 );
  }
} 





void         tbufstream::writedata( char* buf, int size )
{ 
  int         s, actpos2; 

   if (status != 0) return;

   #ifdef dlg_box_h
   if (modus == 1) 
      displaymessage("attempt to write to a file opened only for reading\nfile: %s\n", 2,devicename) ;
   #endif

   actpos2 = 0; 

   while (actpos2 < size) { 
      s = memsize - actmempos; 
      if (s > size - actpos2) 
         s = size - actpos2; 

      memcpy(  zeiger + actmempos, buf + actpos2, s );
      actmempos = actmempos + s; 
      if (actmempos == memsize) { 
         writebuffer();
         actmempos = 0; 
      } 
      else 
         if (actmempos > memsize) 

          #ifdef dlg_box_h
          displaymessage("severe error writing data\nfile: %s\n",2,devicename)
          #endif
          ;
      actpos2 = actpos2 + s; 
   } 
} 


void         tbufstream::done(void)
{ 
   if (modus == 2) {
      writebuffer();
   } /* endif */
   asc_free ( zeiger );
   // fclose(logfile);
} 



  /*  --------------- tfilestream ---------------------  */ 


void         tfilestream::init(void)
{ 
   tbufstream::init();
   actfilepos = 0; 
} 



int         tfilestream::getstreamsize(void)
{ 
   return filesize ( devicename );
}                 

void         tfilestream::openstream(char* name, char mode)
{ 
   if (modus != 0) 
      closestream(); 
   status = 0; 
   modus = mode; 


   // fprintf( logfile, "open file: %s \n", name );
/*
   if (mode == 1) {
      datei.open ( name, ios::in );
   } else {
      datei.open ( name, ios::out );
   }
*/
   if (mode == 1) {
      fp = fopen ( name, filereadmode );
   } else {
      fp = fopen ( name, filewritemode );
   }

   if (fp != NULL) {

     actfilepos = 0;
     datasize = 0;
     if (mode == 1)
       readbuffer();
                
     actmempos = 0;
     status = 0;
     devicename = name;
   } else {

      status = 1;
      #ifdef dlg_box_h
      displaymessage("Could not open file %s: ", 2, name );
      #endif

   } /* endif */
} 






void         tfilestream::seekstream( int newpos, int seekpos )
{ 
   if (status != 0) return;
   if (modus == 2) {
      writebuffer();
   } /* endif */ 
/*   datei.seekg( newpos ); */
   fseek( fp, newpos, seekpos );
   actmempos = 0; 
   actfilepos = newpos; 
   if (modus == 1) 
      readbuffer(); 
}           


void         tfilestream::readbuffer( void )
{ 
   if (status != 0) return;
   /* datasize = datei.read( zeiger, memsize ); */
   datasize = fread( zeiger, 1, memsize, fp);
   status = ferror ( fp );
   actfilepos = actfilepos + datasize;
} 



void         tfilestream::resetstream(void)
{ 
   if (modus == 2) {
      writebuffer();
   }             
   seekstream(0); 
} 



void         tfilestream::writebuffer()
{ 
 /*  result = datai.write (zeiger, actmempos); */
 fwrite( zeiger, 1, actmempos, fp );
 status = ferror ( fp );
 actmempos = 0;
} 


void         tfilestream::closestream(void)
{ 
   if (modus == 2) 
      writebuffer(); 
  
  /* datei.close(); */
//   fprintf( logfile, "close file \n");
  fclose( fp );
   modus = 0; 
   status = 0; 
} 


void         tfilestream::done(void)
{ 
   if (modus != 0) 
      closestream(); 
   tbufstream::done();
} 


  /*  ------------------ tsfilestream ------------------  */ 

/*

void         tsfilestream::seekstream(void)
{ 
   tfilestream::seekstream(startpos + newpos);
} 


void         tsfilestream::openstream( char* name; char mode  )
{ 


  tindexstruct is; 
  int          w; 

   if (modus != 0) 
      closestream(); 
   status = 0; 

   assign(datei,name); 
   reset(&datei,1); 
   if (ioresult != 0) { 
      name = upcasestr(name); 
      assign(datei,"index.dat"); 
      reset(&datei,1); 
      if (ioresult != 0) 
         displaymessage("fatal error 1 at tsfilestream.openstream:",scat("could find neither %s nor index.dat",name),2); 
      do { 
         blockread(datei,is, sizeof(is),w); 
         if (w != sizeof(is)) 
            strcpy(is.filename,""); 
      }  while (!(feof(datei) || (upcasestr(is.filename) == name))); 
      if (upcasestr(is.filename) != name) 
         displaymessage("fatal error 2 at tsfilestream.openstream:",scat("could find neither %s nor entry in index.dat",name),2); 
      startpos = is.posstart; 
      stoppos = is.posstop; 
      inherited(); openstream(is.archivename,mode); 
      seekstream(0); 
   } 
   else { 
      startpos = 0; 
      stoppos = filesize(datei); 
      fclose(datei); 
      inherited(); openstream(name,mode); 
   } 
   status = ioresult; 
   devicename = name; 

} 


void         tsfilestream::readbuffer(void)
{ 
   if (actfilepos + size > stoppos) 
      size = stoppos - actfilepos; 
  tfilestream.readbuffer();
} 


void         tsfilestream::getstreamsize(void)
{ 
   getstreamsize = stoppos - startpos; 
} 



*/



#ifndef converter
const double pi = 3.14159265;

void  rotatepict45 ( void *s, void *d )
{
    struct timg {
       int temp; 
       char pix[unitsizex+1][unitsizey+1];
    } ;
    timg* dest = (timg*) d;
    timg* sorc = (timg*) s;
   {
   
      dest->temp = sorc->temp;
      memset ( dest->pix, 0xff, sizeof ( dest->pix ));
   
      for (int x = 0; x <= unitsizex; x++) {
         double dx = x - 15;
         for (int y = 0; y <= unitsizey; y++ ) {
            if ( sorc->pix[ x][y] != 255 ) {
               double dy = y - 15;
               double alpha ;
               double radius = sqrt ( dx*dx + dy*dy );
               if ( dx > 0 ) {
                  if ( dy >= 0 )
                    alpha =  atan ( dy / dx );
                  else
                    if ( dy < 0 )
                      alpha =  pi * 2 - atan ( -dy / dx );
               } else
                  if ( dx < 0 ) {
                     if ( dy >= 0 )
                       alpha =  pi - atan ( dy / -dx );
                     if ( dy < 0 )
                       alpha =  pi + atan ( dy / dx );
                  } else {
                    if ( dy > 0 )
                       alpha = pi/2;
                    if ( dy < 0 )
                       alpha = 3*pi/2;
                    if ( dy  == 0 ) {
                       dest->pix[15][15] = sorc->pix[15][15];
                       break;
                    }   
                  }
               alpha -= pi/4;
               if ( alpha < 0  )
                  alpha+= pi*2;
       
               double ny;
               double nx;
               if ( alpha < pi/2 ) {
                  ny = radius * sin ( alpha );
                  nx = radius * cos ( alpha );
               } else
                 if ( alpha == pi/2 ) {
                    ny = radius;
                    nx = 0;
                 } else
                    if ( alpha < pi ) {
                       ny = radius * sin ( pi - alpha );
                       nx = -radius * cos ( pi - alpha );
                    } else
                       if ( alpha == pi/2 ) {
                          ny = radius;
                          nx = 0;
                       } else
                          if ( alpha < pi ) {
                             ny = radius * sin ( alpha );
                             nx = radius * cos ( alpha );
                          } else
                             if ( alpha == pi ) {
                                ny = 0;
                                nx = -radius;
                             } else
                                if ( alpha < 3*pi/2 ) {
                                   ny = -radius * sin ( alpha - pi );
                                   nx = -radius * cos ( alpha - pi );
                                } else
                                   if ( alpha == 3*pi/2 ) {
                                      ny = -radius;
                                      nx = 0;
                                   } else {
                                       ny = -radius * sin ( 2*pi - alpha  );
                                       nx = radius * cos ( 2*pi - alpha  );
                                   }
                if ( nx > 0 )
                   nx += 0.5;
                if ( nx < 0 )
                   nx -= 0.5;
                if ( ny > 0 )
                   ny += 0.5;
                if ( ny < 0 )
                   ny -= 0.5;
       
                int inx = (int)nx;
                int iny = (int)ny;
                if ( inx > 15 )
                   inx = 15;
                if ( inx < -15 )
                   inx = -15;
                if ( iny > 15 )
                   iny = 15;
                if ( iny < -15 )
                   iny = -15;
       
                dest->pix[ 15 + inx ][ 15 + iny ] = sorc->pix[ x][y];
            }
         } /* endfor */
      } /* endfor */
   }


   for ( int m = 0; m <2; m++ ) {
      
       int col[4];
       int pnum;
       for (int y = 0; y <= unitsizey; y++) 
          for (int x = 0; x <= unitsizex; x++) {
             if ( dest->pix[ x ][ y ] == 255 ) {
                pnum = 0;
                if ( y > 0 ) {
                   col[ pnum ] = dest->pix[ x ][ y-1 ];
                   if ( col[ pnum ] != 255 ) 
                      pnum++;
                 }
                if ( x > 0 ) {
                   col[ pnum ] = dest->pix[ x-1 ][ y ];
                   if ( col[ pnum ] != 255 ) 
                      pnum++;
                 }
                if ( y < unitsizey  ) {
                   col[ pnum ] = dest->pix[ x ][ y+1 ];
                   if ( col[ pnum ] != 255 ) 
                      pnum++;
                 }
                if ( x < unitsizex ) {
                   col[ pnum ] = dest->pix[ x+1 ][ y ];
                   if ( col[ pnum ] != 255 ) 
                      pnum++;
                 }
                 /* if (pnum == 1)
                    buffer[ getbufpos( x, y ) ] = col[0];
                 else
                    if (pnum == 2) 
                       dest->pix[ x ][ y ] = mix2colors ( col[0], col[1] );
                    else 
                       */
                      if (pnum == 3) 
                         dest->pix[ x ][ y ] = mix3colors ( col[0], col[1], col[2] );
                      else 
                         if (pnum == 4) 
                            dest->pix[ x ][ y ]= mix4colors ( col[0], col[1], col[2], col[3]  );
             }
          } /* endfor */
       if ( m == 0 ) 
          dest->pix[ 17 ][ 17 ] = dest->pix[ 16 ][ 16 ];

   }

}

union tpix {
  struct { char r,g,b,a; } s;
  int all;
};

typedef tpix timage[ fieldxsize ][ fieldysize ];

int getimagepixel ( void* image, int x, int y )
{
   int xs, ys;
   getpicsize ( image, xs, ys );


   y += ys/2;
   x += xs/2;
   if ( x < 0  || x >= xs || y < 0 || y >= ys )
      return -1;
   else {
      char* pc = (char*) image;
      return pc[ 4 + y * xs + x];
   }
}


char* rotatepict ( void* image, int organgle )
{
   float angle = ((float)organgle) / 360 * 2 * pi + pi;

   char* dst = new char[ imagesize ( 0, 0, fieldxsize, fieldysize ) ];
   dst[0] = fieldxsize-1;   
   dst[1] = 0;

   dst[2] = fieldysize-1;
   dst[3] = 0;
  
   char* pnt  = dst + 4;

   for ( int y = 0; y < fieldysize; y++ ) {
      for ( int x = 0; x < fieldxsize; x++ ) {
         int dx = x - fieldxsize/2 ;
         int dy = fieldysize/2 - y;
         float nx, ny;
         if ( organgle != 0 && organgle != -180 && organgle != 180) {
            float wnk ;
            if ( dx  ) 
               wnk = atan2 ( dy, dx );
            else
               if ( dy > 0 )
                  wnk = pi/2;
               else
                  wnk = -pi/2;
   
            wnk -= angle;
            float radius = sqrt ( dx * dx + dy * dy );
   
            nx = radius * cos ( wnk );
            ny = radius * sin ( wnk );
         } else 
            if ( organgle == 0 ) {
               nx = -dx;
               ny = -dy;
            } else
               if ( organgle == 180 || organgle == -180) {
                  nx = dx;
                  ny = dy;
               }
         

         int newpix = getimagepixel ( image, (int)-nx, (int)ny );
         if ( newpix == -1 )
            *pnt = 255;
         else
            *pnt = newpix;

         pnt++;
      }
   }

   return dst;
}


#endif



const int vehicle_version = 3;
const int building_version = 2;
const int object_version = 1;
const int terrain_version = 1;
const int technology_version = 1;

void* leergui = NULL;
void* removegui = NULL;


void* generate_vehicle_gui_build_icon ( pvehicletype tnk )
{

   int wd;
   int hg;
   getpicsize ( leergui, wd, hg );

   int minx = 1000;
   int miny = 1000;
   int maxx = 0;
   int maxy = 0;

   tvirtualdisplay vdsp ( 500, 500 );

    bar ( 0, 0, 450, 450, 255 );
  

    minx = 0;
    miny = 0;
    putspriteimage ( 0, 0, tnk->picture[0] );
    maxx= fieldxsize;
    maxy= fieldysize;

   int sze = imagesize ( minx, miny, maxx, maxy );
   void* buf = new char [ sze ];
   getimage ( minx, miny, maxx, maxy, buf );

   while ( (maxx - minx + 1 > wd ) || ( maxy - miny + 1 > hg )) {
      void* temp = halfpict ( buf );

      char* dst = (char*) buf;
      char* src = (char*) temp;

      for ( int i = 0; i < sze; i++ )
         *(dst++) = *(src++);

      minx = 0;
      miny = 0;
      getpicsize ( buf, maxx, maxy );

   } /* endwhile */

   putimage ( 0, 0, leergui );
   putspriteimage ( (wd - maxx) / 2, (hg - maxy) / 2, buf );

   char* newbuildingpic = new char [ imagesize ( 0, 0, wd-1, hg-1 ) ];
   getimage ( 0, 0, wd-1, hg-1, newbuildingpic );
   delete[] buf;

   return newbuildingpic;
}



pvehicletype   loadvehicletype(char *       name)
{
   tnfilestream stream ( name, 1 );
   return loadvehicletype ( &stream );
}


pvehicletype   loadvehicletype( pnstream stream )
{ 
   int version = stream->readInt();
   if ( version <= vehicle_version && version >= 1) {

      int   j;

      pvehicletype fztn = new tvehicletype;
   
      fztn->name = (char*) stream->readInt();
      fztn->description = (char*) stream->readInt();
      fztn->infotext = (char*) stream->readInt();

      if ( version <= 2 ) {
         fztn->oldattack.weaponcount = stream->readChar();
         for ( j = 0; j< 8; j++ ) {
            fztn->oldattack.waffe[j].typ = stream->readWord();
            fztn->oldattack.waffe[j].targ = stream->readChar();
            fztn->oldattack.waffe[j].sourceheight = stream->readChar();
            fztn->oldattack.waffe[j].maxdistance = stream->readWord();
            fztn->oldattack.waffe[j].mindistance = stream->readWord();
            fztn->oldattack.waffe[j].count = stream->readChar();
            fztn->oldattack.waffe[j].maxstrength = stream->readChar();
            fztn->oldattack.waffe[j].minstrength = stream->readChar();
         }
      }

      fztn->production.energy   = stream->readWord();
      fztn->production.material = stream->readWord();
      fztn->armor = stream->readWord();
      for ( j = 0; j < 8; j++ ) 
          fztn->picture[j] = (void*)  stream->readInt();
      
      fztn->height = stream->readChar();
      fztn->researchid = stream->readWord();
      if ( version <= 2 ) {
         fztn->_terrain = stream->readInt();
         fztn->_terrainreq = stream->readInt();
         fztn->_terrainkill = stream->readInt();
      }
      fztn->steigung = stream->readChar();
      fztn->jamming = stream->readChar();
      fztn->view = stream->readWord();
      fztn->wait = stream->readChar();

      if ( version <= 2 )
         stream->readChar (); // dummy 

      fztn->loadcapacity = stream->readWord();
      fztn->maxunitweight = stream->readWord();
      fztn->loadcapability = stream->readChar();
      fztn->loadcapabilityreq = stream->readChar();
      fztn->loadcapabilitynot = stream->readChar();
      fztn->id = stream->readWord();
      fztn->tank = stream->readInt();
      fztn->fuelconsumption = stream->readWord();
      fztn->energy = stream->readInt();
      fztn->material = stream->readInt();
      fztn->functions = stream->readInt();

      for ( j = 0; j < 8; j++ ) 
         fztn->movement[j] = stream->readChar();
      

      fztn->movemalustyp = stream->readChar();

      if ( version <= 2 ) 
         for ( j = 0; j < 9; j++ )
             stream->readWord( ); // dummy1

      fztn->classnum = stream->readChar();
      for ( j = 0; j < 8; j++ ) 
          fztn->classnames[j] = (char*) stream->readInt();

      for ( j = 0; j < 8; j++ ) {
         int k;
         for ( k = 0; k < 8; k++ ) 
            fztn->classbound[j].weapstrength[k] = stream->readWord();
         
         if ( version <= 2 ) 
            stream->readWord (  ); // dummy2

         fztn->classbound[j].armor = stream->readWord();
         fztn->classbound[j].techlevel = stream->readWord();
         for ( k = 0; k < 4; k++ ) 
            fztn->classbound[j].techrequired[k] = stream->readWord();
         
         fztn->classbound[j].eventrequired = stream->readChar();
         fztn->classbound[j].vehiclefunctions = stream->readInt();
         if ( version <= 2 )
            stream->readChar( ); // dummy
      }

      fztn->maxwindspeedonwater = stream->readChar();
      fztn->digrange = stream->readChar();
      fztn->initiative = stream->readInt();
      fztn->_terrainnot = stream->readInt();
      fztn->_terrainreq1 = stream->readInt();
      fztn->objectsbuildablenum = stream->readInt();
      fztn->objectsbuildableid = (int*) stream->readInt();
      fztn->weight = stream->readInt();
      fztn->terrainaccess = (pterrainaccess) stream->readInt();
      fztn->bipicture = stream->readInt();
      fztn->vehiclesbuildablenum = stream->readInt();
      fztn->vehiclesbuildableid = (int*) stream->readInt();
      fztn->buildicon = (void*) stream->readInt();
      fztn->buildingsbuildablenum = stream->readInt();
      fztn->buildingsbuildable = (pbuildrange) stream->readInt();
      fztn->weapons = (UnitWeapon*) stream->readInt();
      fztn->autorepairrate = stream->readInt();
      if ( version <= 2 )
         stream->readInt( ); // dummy

      if (fztn->name)
         stream->readpchar( &fztn->name );

      if (fztn->description)
         stream->readpchar( &fztn->description );

      if (fztn->infotext)
         stream->readpchar( &fztn->infotext );

      int i;
      for ( i=0;i<8  ;i++ ) 
         if ( fztn->classnames[i] )
            stream->readpchar( &(fztn->classnames[i]) );
   
      if ( fztn->functions & cfautorepair )
         if ( !fztn->autorepairrate )
            fztn->autorepairrate = autorepairdamagedecrease; // which is 10
            
      int size;
      for (i=0;i<8  ;i++ ) 
         if ( fztn->picture[i] ) 
            if ( fztn->bipicture <= 0 )
               stream->readrlepict ( &fztn->picture[i], false, &size);
            else
               loadbi3pict_double ( fztn->bipicture, &fztn->picture[i], gameoptions.bi3.interpolate.units );

      if ( fztn->objectsbuildablenum ) {
         fztn->objectsbuildableid = new int [ fztn->objectsbuildablenum ];
         for ( i = 0; i < fztn->objectsbuildablenum; i++ ) 
            fztn->objectsbuildableid[i] = stream->readInt ( );
      }

      if ( fztn->vehiclesbuildablenum ) {
         fztn->vehiclesbuildableid = new int [ fztn->vehiclesbuildablenum ];
         for ( i = 0; i < fztn->vehiclesbuildablenum; i++ ) 
            fztn->vehiclesbuildableid[i] = stream->readInt() ;
      }

      fztn->weapons = new UnitWeapon;
      if ( fztn->weapons && version > 1) {
         fztn->weapons->count = stream->readInt();
         for ( j = 0; j< 16; j++ ) {
            fztn->weapons->weapon[j].set ( stream->readInt() );
            fztn->weapons->weapon[j].targ = stream->readInt();
            fztn->weapons->weapon[j].sourceheight = stream->readInt();
            fztn->weapons->weapon[j].maxdistance = stream->readInt();
            fztn->weapons->weapon[j].mindistance = stream->readInt();
            fztn->weapons->weapon[j].count = stream->readInt();
            fztn->weapons->weapon[j].maxstrength = stream->readInt();
            fztn->weapons->weapon[j].minstrength = stream->readInt();

            for ( int k = 0; k < 13; k++ ) 
               fztn->weapons->weapon[j].efficiency[k] = stream->readInt();
            
            fztn->weapons->weapon[j].targets_not_hittable = stream->readInt();

            if ( version <= 2 )
               for ( int l = 0; l < 9; l++ ) 
                  stream->readInt(); // dummy
            
         }

         if ( version <= 2 )
            for ( int m = 0; m< 10; m++ ) 
               stream->readInt(); // dummy 
         
      } else {
         fztn->weapons->count = fztn->oldattack.weaponcount;
         for ( i = 0; i < fztn->oldattack.weaponcount; i++ ) {
            fztn->weapons->weapon[i].set ( fztn->oldattack.waffe[i].typ );
            fztn->weapons->weapon[i].targ  = fztn->oldattack.waffe[i].targ;
            fztn->weapons->weapon[i].sourceheight  = fztn->oldattack.waffe[i].sourceheight;
            fztn->weapons->weapon[i].maxdistance  = fztn->oldattack.waffe[i].maxdistance;
            fztn->weapons->weapon[i].mindistance  = fztn->oldattack.waffe[i].mindistance;
            fztn->weapons->weapon[i].count  = fztn->oldattack.waffe[i].count;
            fztn->weapons->weapon[i].maxstrength  = fztn->oldattack.waffe[i].maxstrength;
            fztn->weapons->weapon[i].minstrength  = fztn->oldattack.waffe[i].minstrength;
            for ( int j = 0; j < 13; j++ )
               fztn->weapons->weapon[i].efficiency[j] = 100;

         }
      }
   
      #ifndef converter
         if ( fztn->bipicture <= 0 ) {
            TrueColorImage* zimg = zoomimage ( fztn->picture[0], fieldxsize, fieldysize, pal, 0 );
            void* pic = convertimage ( zimg, pal ) ;
            for (  i = 0; i < 6; i++ )
                fztn->picture[i] = rotatepict ( pic, directionangle[i] ); // -2 * pi / 6 * i
            delete[] pic;
            delete zimg;
         } else {
            for (  i = 1; i < 6; i++ )
               fztn->picture[i] = rotatepict ( fztn->picture[0], directionangle[i] ); // -2 * pi / 6 * i
         }
      #endif
   
      if ( fztn->terrainaccess ) {
         fztn->terrainaccess = new tterrainaccess;
         stream->readdata2 ( *(fztn->terrainaccess) );
      } else {
         fztn->terrainaccess = new tterrainaccess;
         fztn->terrainaccess->terrain.set ( fztn->_terrain, 0 );
         fztn->terrainaccess->terrainreq.set ( fztn->_terrainreq, 0 );
         fztn->terrainaccess->terrainnot.set ( fztn->_terrainnot, 0 );
         fztn->terrainaccess->terrainkill.set ( fztn->_terrainkill, 0 );
      }
   
      if ( fztn->buildingsbuildablenum ) {
         fztn->buildingsbuildable = new tbuildrange[fztn->buildingsbuildablenum];
         for ( i = 0; i < fztn->buildingsbuildablenum; i++ )
            stream->readdata2( fztn->buildingsbuildable[i] );
      }


      #ifndef converter
       fztn->buildicon = generate_vehicle_gui_build_icon ( fztn );
      #else
       fztn->buildicon = NULL;
      #endif
      return fztn;
   } else
      return NULL;
} 




void writevehicle( pvehicletype fztn, pnstream stream )
{
   const int  one  = 1;
   const int  zero = 0;

   int i,j;

   stream->writeInt ( vehicle_version ); 

   if ( fztn->name )
      stream->writeInt( one );  
   else
      stream->writeInt( zero );  

   if ( fztn->description )
      stream->writeInt( one );  
   else
      stream->writeInt( zero );  

   if ( fztn->infotext )
      stream->writeInt( one );  
   else
      stream->writeInt( zero );  

   stream->writeWord( fztn->production.energy ); 
   stream->writeWord( fztn->production.material );
   stream->writeWord( fztn->armor );

   for ( j = 0; j < 8; j++ )
      if ( fztn->picture[j] )
         stream->writedata2( one ); 
      else
         stream->writedata2( zero ); 
   
   stream->writeChar( fztn->height );
   stream->writeWord(fztn->researchid);
   stream->writeChar(fztn->steigung);
   stream->writeChar(fztn->jamming);
   stream->writeWord(fztn->view);
   stream->writeChar(fztn->wait);
   stream->writeWord(fztn->loadcapacity);
   stream->writeWord(fztn->maxunitweight);
   stream->writeChar(fztn->loadcapability);
   stream->writeChar(fztn->loadcapabilityreq);
   stream->writeChar(fztn->loadcapabilitynot);
   stream->writeWord(fztn->id );
   stream->writeInt(fztn->tank );
   stream->writeWord(fztn->fuelconsumption );
   stream->writeInt(fztn->energy );
   stream->writeInt(fztn->material );
   stream->writeInt(fztn->functions );
   for ( j = 0; j < 8; j++ ) 
       stream->writeChar( fztn->movement[j] );
   

   stream->writeChar(fztn->movemalustyp );
   stream->writeChar(fztn->classnum );
   for ( j = 0; j < 8; j++ ) 
      if ( fztn->classnames[j] )
          stream->writedata2( one ); 
      else
          stream->writedata2( zero ); 
      

   for ( j = 0; j < 8; j++ ) {
      int k;
      for ( k = 0; k < 8; k++ ) 
         stream->writeWord(fztn->classbound[j].weapstrength[k]);
      
      stream->writeWord(fztn->classbound[j].armor);
      stream->writeWord(fztn->classbound[j].techlevel );
      for ( k = 0; k < 4; k++ ) 
         stream->writeWord(fztn->classbound[j].techrequired[k]);
      
      stream->writeChar(fztn->classbound[j].eventrequired );
      stream->writeInt(fztn->classbound[j].vehiclefunctions );
   }

   stream->writeChar(fztn->maxwindspeedonwater );
   stream->writeChar(fztn->digrange );
   stream->writeInt(fztn->initiative );
   stream->writeInt(fztn->_terrainnot );
   stream->writeInt(fztn->_terrainreq1 );
   stream->writeInt(fztn->objectsbuildablenum );

   if ( fztn->objectsbuildableid ) 
      stream->writedata2( one ); 
   else
      stream->writedata2( zero ); 

   stream->writeInt(fztn->weight );
   if ( fztn->terrainaccess )
      stream->writedata2( one ); 
   else
      stream->writedata2( zero ); 

   stream->writeInt(fztn->bipicture );
   stream->writeInt(fztn->vehiclesbuildablenum );
   if ( fztn->vehiclesbuildableid )
      stream->writedata2( one ); 
   else
      stream->writedata2( zero ); 

   if ( fztn->buildicon )
      stream->writedata2( one ); 
   else
      stream->writedata2( zero ); 

   stream->writeInt(fztn->buildingsbuildablenum );
   if ( fztn->buildingsbuildable )
      stream->writedata2( one ); 
   else
      stream->writedata2( zero ); 

   if ( fztn->weapons )
      stream->writedata2( one ); 
   else
      stream->writedata2( zero ); 

   stream->writeInt(fztn->autorepairrate );

   if (fztn->name)
      stream->writepchar( fztn->name );

   if (fztn->description)
      stream->writepchar( fztn->description );

   if (fztn->infotext)
      stream->writepchar( fztn->infotext );

   for (i=0; i<8; i++)
      if ( fztn->classnames[i] )
         stream->writepchar( fztn->classnames[i] );

   if ( fztn->bipicture <= 0 )
      for (i=0;i<8  ;i++ ) 
         if ( fztn->picture[i] ) 
            stream->writedata ( fztn->picture[i], getpicsize2 ( fztn->picture[i] ) );

   for ( i = 0; i < fztn->objectsbuildablenum; i++ )
      stream->writeInt ( fztn->objectsbuildableid[i] );

   for ( i = 0; i < fztn->vehiclesbuildablenum; i++ )
      stream->writeInt ( fztn->vehiclesbuildableid[i] );

   stream->writeInt(fztn->weapons->count ); 
   for ( j = 0; j< 16; j++ ) {
      stream->writeInt(fztn->weapons->weapon[j].gettype( )); 
      stream->writeInt(fztn->weapons->weapon[j].targ); 
      stream->writeInt(fztn->weapons->weapon[j].sourceheight ); 
      stream->writeInt(fztn->weapons->weapon[j].maxdistance ); 
      stream->writeInt(fztn->weapons->weapon[j].mindistance ); 
      stream->writeInt(fztn->weapons->weapon[j].count ); 
      stream->writeInt(fztn->weapons->weapon[j].maxstrength ); 
      stream->writeInt(fztn->weapons->weapon[j].minstrength ); 

      for ( int k = 0; k < 13; k++ ) 
         stream->writeInt(fztn->weapons->weapon[j].efficiency[k] ); 

      stream->writeInt(fztn->weapons->weapon[j].targets_not_hittable ); 
   }

   if ( fztn->terrainaccess ) 
      stream->writedata2 ( *(fztn->terrainaccess) );
   
   for ( i = 0; i < fztn->buildingsbuildablenum; i++ ) {
      stream->writeInt( fztn->buildingsbuildable[i].from );
      stream->writeInt( fztn->buildingsbuildable[i].to );
   }

}





ptechnology       loadtechnology(char *       name)
{
   tnfilestream stream ( name, 1 ); 
   return loadtechnology ( &stream );
}


ptechnology       loadtechnology( pnstream stream )
{ 
   int          w;

   int version;
   stream->readdata2 ( version );
   if ( version == technology_version ) {

      ptechnology pt =  new ttechnology;
   
      stream->readdata ( pt, sizeof(*pt) ); 
      if ( pt->name )
         stream->readpchar( &pt->name );
      if ( pt->infotext )
         stream->readpchar( &pt->infotext );
      if ( pt->icon )
         stream->readrlepict ( &pt->icon,false,&w);
      if ( pt->pictfilename )
         stream->readpchar( &pt->pictfilename );
   
      pt->lvl = -1;

      return pt; 
   } else
      return NULL;
} 

void writetechnology ( ptechnology tech, pnstream stream )
{
   stream->writedata2 ( technology_version );

   stream->writedata2 ( *tech );
 
   if (tech->name)
      stream->writepchar( tech->name );
   if (tech->infotext)
      stream->writepchar( tech->infotext );
   if (tech->icon)
      stream->writerlepict( tech->icon );
   if ( tech->pictfilename )
      stream->writepchar( tech->pictfilename );
} 






void* generate_building_gui_build_icon ( pbuildingtype bld )
{

   int wd;
   int hg;
   getpicsize ( leergui, wd, hg );

   int minx = 1000;
   int miny = 1000;
   int maxx = 0;
   int maxy = 0;

   tvirtualdisplay vdsp ( 500, 500 );

    bar ( 0, 0, 450, 450, 255 );
  
    for (int y = 0; y <= 5; y++)
       for (int x = 0; x <= 3; x++)
          if (bld->getpicture(x,y) ) {
             int xp = fielddistx * x  + fielddisthalfx * ( y & 1);
             int yp = fielddisty * y ;
             if ( xp < minx )
                minx = xp;
             if ( yp < miny )
                miny = yp;
             if ( xp > maxx )
                maxx = xp;
             if ( yp > maxy )
                maxy = yp;

             putspriteimage ( xp, yp, bld->getpicture(x,y) );
          }
   maxx += fieldxsize;
   maxy += fieldysize;

   int sze = imagesize ( minx, miny, maxx, maxy );
   char* buf = new char [ sze ];
   getimage ( minx, miny, maxx, maxy, buf );

/*
   while ( (maxx - minx + 1 > wd ) || ( maxy - miny + 1 > hg )) {
      void* temp = halfpict ( buf );

      char* dst = (char*) buf;
      char* src = (char*) temp;

      for ( int i = 0; i < sze; i++ )
         *(dst++) = *(src++);

      minx = 0;
      miny = 0;
      getpicsize ( buf, maxx, maxy );

   } 
*/
   TrueColorImage* img = zoomimage ( buf, wd, hg, pal, 1 );
   delete[] buf;

   buf = convertimage ( img, pal );
   delete img;
   getpicsize ( buf, maxx, maxy );

   putimage ( 0, 0, leergui );
   putspriteimage ( (wd - maxx) / 2, (hg - maxy) / 2, buf );

   char* newbuildingpic = new char [ imagesize ( 0, 0, wd-1, hg-1 ) ];
   getimage ( 0, 0, wd-1, hg-1, newbuildingpic );
   delete[] buf;

   return newbuildingpic;


}



void* generate_object_gui_build_icon ( pobjecttype obj, int remove )
{

   int wd;
   int hg;
   getpicsize ( leergui, wd, hg );

   int minx = 1000;
   int miny = 1000;
   int maxx = 0;
   int maxy = 0;

   tvirtualdisplay vdsp ( 500, 500 );

    bar ( 0, 0, 450, 450, 255 );
  

    minx = 0;
    miny = 0;
    obj->display( 0, 0 );
    maxx= fieldxsize;
    maxy= fieldysize;

   int sze = imagesize ( minx, miny, maxx, maxy );
   void* buf = new char [ sze ];
   getimage ( minx, miny, maxx, maxy, buf );

   while ( (maxx - minx + 1 > wd ) || ( maxy - miny + 1 > hg )) {
      void* temp = halfpict ( buf );

      char* dst = (char*) buf;
      char* src = (char*) temp;

      for ( int i = 0; i < sze; i++ )
         *(dst++) = *(src++);

      minx = 0;
      miny = 0;
      getpicsize ( buf, maxx, maxy );

   } /* endwhile */

   putimage ( 0, 0, leergui );
   putspriteimage ( (wd - maxx) / 2, (hg - maxy) / 2, buf );

   if ( remove )
     putspriteimage ( (wd-18)/2, (hg-18)/2, removegui );

   void* newbuildingpic = new char [ imagesize ( 0, 0, wd-1, hg-1 ) ];
   getimage ( 0, 0, wd-1, hg-1, newbuildingpic );
   delete[] buf;

   return newbuildingpic;
}

void loadguipictures( void )
{
   if ( !leergui ) {
      tnfilestream stream ( "leergui.raw", 1 );
      int sze;
      stream.readrlepict ( &leergui, 0, &sze );
   }
   if ( !removegui ) {
      tnfilestream stream ( "guiremov.raw", 1 );
      int sze;
      stream.readrlepict ( &removegui, 0, &sze );
   }
}


pbuildingtype       loadbuildingtype(char *       name)
{
   tnfilestream stream ( name, 1 ); 
   return loadbuildingtype ( &stream );
}


pbuildingtype       loadbuildingtype( pnstream stream )
{ 
   int v, w, x, y;

   int version;
   stream->readdata2 ( version );
   if ( version <= building_version && version >= 1) {

      pbuildingtype pgbt = new tbuildingtype;
   
      for ( v = 0; v < cwettertypennum; v++ )
         for ( w = 0; w < maxbuildingpicnum; w++ )
            for ( x = 0; x < 4; x++ )
               for ( y = 0; y < 6 ; y++ ) 
                   pgbt->w_picture[v][w][x][y] = (void*)stream->readInt( );

      for ( v = 0; v < cwettertypennum; v++ )
         for ( w = 0; w < maxbuildingpicnum; w++ )
            for ( x = 0; x < 4; x++ )
               for ( y = 0; y < 6 ; y++ ) 
                   pgbt->bi_picture[v][w][x][y] = stream->readInt( );
               
      pgbt->entry.x = stream->readInt( );
      pgbt->entry.y = stream->readInt( );
      pgbt->powerlineconnect.x = stream->readInt( );
      pgbt->powerlineconnect.y = stream->readInt( );
      pgbt->pipelineconnect.x = stream->readInt( );
      pgbt->pipelineconnect.y = stream->readInt( );

      pgbt->id = stream->readInt( );
      pgbt->name = (char*) stream->readInt( );
      pgbt->armor = stream->readInt( );
      pgbt->jamming = stream->readInt( );
      pgbt->view = stream->readInt( );
      pgbt->loadcapacity = stream->readInt( );
      pgbt->loadcapability = stream->readChar( );
      pgbt->unitheightreq = stream->readChar( );
      pgbt->productioncost.material = stream->readInt( );
      pgbt->productioncost.fuel = stream->readInt( );
      pgbt->special = stream->readInt( );
      pgbt->technologylevel = stream->readChar( );
      pgbt->researchid = stream->readChar( );

      stream->readdata2 ( pgbt->terrainaccess );    // !!!!byteorder!!!!

      pgbt->construction_steps = stream->readInt( );
      pgbt->maxresearchpoints = stream->readInt( );
      pgbt->_tank.a.energy = stream->readInt( );
      pgbt->_tank.a.material = stream->readInt( );
      pgbt->_tank.a.fuel = stream->readInt( );
      pgbt->maxplus.a.energy = stream->readInt( );
      pgbt->maxplus.a.material = stream->readInt( );
      pgbt->maxplus.a.fuel = stream->readInt( );
      pgbt->efficiencyfuel = stream->readInt( );
      pgbt->efficiencymaterial = stream->readInt( );
      pgbt->guibuildicon = (char*) stream->readInt( );
      pgbt->terrain_access = (pterrainaccess) stream->readInt( );

      pgbt->_bi_maxstorage.a.energy = stream->readInt( );
      pgbt->_bi_maxstorage.a.material = stream->readInt( );
      pgbt->_bi_maxstorage.a.fuel = stream->readInt( );

      pgbt->buildingheight = stream->readInt( );
      pgbt->unitheight_forbidden = stream->readInt( );
      pgbt->externalloadheight = stream->readInt( );

      if ( version >= 2 ) {
         for ( x = 0; x < 4; x++ )
            for ( y = 0; y < 6; y++ )
                pgbt->destruction_objects[x][y] = stream->readInt( );
      } else {
         for ( w = 0; w < 9; w++ )
             stream->readInt( );     // dummy

         for ( x = 0; x < 4; x++ )
            for ( y = 0; y < 6; y++ )
                pgbt->destruction_objects[x][y] = 0;
      }

      if ( pgbt->name )
         stream->readpchar ( &pgbt->name );
   
      for ( int k = 0; k < maxbuildingpicnum ; k++)
         for ( int j = 0; j <= 5; j++) 
            for ( int i = 0; i <= 3; i++) 
               for ( int w = 0; w < cwettertypennum; w++ )
                 if ( pgbt->w_picture[w][k][i][j] ) 
                    if ( pgbt->bi_picture[w][k][i][j] == -1 ) {
                       int sz;
                       stream->readrlepict ( &pgbt->w_picture[w][k][i][j], false, &sz ); 
                     } else 
                        loadbi3pict_double ( pgbt->bi_picture[w][k][i][j], &pgbt->w_picture[w][k][i][j], gameoptions.bi3.interpolate.buildings );
                     
                  
       pgbt->terrain_access = &pgbt->terrainaccess; 
   
     #ifdef converter
      pgbt->guibuildicon = NULL;
     #else
      pgbt->guibuildicon = generate_building_gui_build_icon ( pgbt );
     #endif

      return pgbt; 
   } else
      return NULL;
} 


void writebuildingtype ( pbuildingtype bld, pnstream stream )
{
   int v,w,x,y;

   stream->writedata2 ( building_version );

   for ( v = 0; v < cwettertypennum; v++ )
      for ( w = 0; w < maxbuildingpicnum; w++ )
         for ( x = 0; x < 4; x++ )
            for ( y = 0; y < 6 ; y++ ) 
                stream->writeInt ( (int) bld->w_picture[v][w][x][y] );

   for ( v = 0; v < cwettertypennum; v++ )
      for ( w = 0; w < maxbuildingpicnum; w++ )
         for ( x = 0; x < 4; x++ )
            for ( y = 0; y < 6 ; y++ ) 
                stream->writeInt ( bld->bi_picture[v][w][x][y] );
            
   stream->writeInt ( bld->entry.x );
   stream->writeInt ( bld->entry.y );
   stream->writeInt ( bld->powerlineconnect.x );
   stream->writeInt ( bld->powerlineconnect.y );
   stream->writeInt ( bld->pipelineconnect.x );
   stream->writeInt ( bld->pipelineconnect.y );

   stream->writeInt ( bld->id );
   stream->writeInt ( (int) bld->name );
   stream->writeInt ( bld->armor );
   stream->writeInt ( bld->jamming );
   stream->writeInt ( bld->view );
   stream->writeInt ( bld->loadcapacity );
   stream->writeChar ( bld->loadcapability );
   stream->writeChar ( bld->unitheightreq );
   stream->writeInt ( bld->productioncost.material );
   stream->writeInt ( bld->productioncost.fuel );
   stream->writeInt ( bld->special );
   stream->writeChar ( bld->technologylevel );
   stream->writeChar ( bld->researchid );

   stream->writedata2 ( bld->terrainaccess );    // !!!!byteorder!!!!

   stream->writeInt ( bld->construction_steps );
   stream->writeInt ( bld->maxresearchpoints );
   stream->writeInt ( bld->_tank.a.energy );
   stream->writeInt ( bld->_tank.a.material );
   stream->writeInt ( bld->_tank.a.fuel );
   stream->writeInt ( bld->maxplus.a.energy );
   stream->writeInt ( bld->maxplus.a.material );
   stream->writeInt ( bld->maxplus.a.fuel );
   stream->writeInt ( bld->efficiencyfuel );
   stream->writeInt ( bld->efficiencymaterial );
   stream->writeInt ( (int) bld->guibuildicon );
   stream->writeInt ( (int) bld->terrain_access );

   stream->writeInt ( bld->_bi_maxstorage.a.energy );
   stream->writeInt ( bld->_bi_maxstorage.a.material );
   stream->writeInt ( bld->_bi_maxstorage.a.fuel );

   stream->writeInt ( bld->buildingheight );
   stream->writeInt ( bld->unitheight_forbidden );
   stream->writeInt ( bld->externalloadheight );

   for ( x = 0; x < 4; x++ )
      for ( y = 0; y < 6; y++ )
          stream->writeInt ( bld->destruction_objects[x][y] );

   if ( bld->name )
      stream->writepchar ( bld->name );

    for (int k = 0; k < maxbuildingpicnum; k++)
       for (int j = 0; j <= 5; j++)
          for (int i = 0; i <= 3; i++)
             for ( int w = 0; w < cwettertypennum; w++ )
                if (bld->w_picture[w][k][i][j] ) 
                   if ( bld->bi_picture[w][k][i][j] == -1 )
                       stream->writedata( bld->w_picture[w][k][i][j],fieldsize);
}


void generatedirecpict ( void* orgpict, void* direcpict )
{
  int t, u;

   tvirtualdisplay vfb ( 100, 100 );
   putspriteimage ( 10, 10, orgpict );

   char *b = (char*) direcpict;

   for (t = 1; t < 20; t++) {
       for (u = 20-t; u < 20+t; u++) {
          *b = getpixel(  10 + u, 9 + t);
          b++;
       }
    }
    
    for (t = 20; t > 0; t-- ) {
       for (u =20-t; u<= 19 + t; u++) {
          *b = getpixel(  10 + u, 10 + 39 - t );
          b++;
       }
    }

}



extern dacpalette256 pal;

#define sqr(a) (a)*(a)


void generateaveragecolprt ( int x1, int y1, int x2, int y2, void* buf, char* pix1 )
{
   word *w = (word*) buf;
   // int size = ( w[0] + 1) * (w[1] + 1);


   char *c ; // = (char*) buf;
             // c+=4;

   int pixnum = 0;

   int i,j;
   int r=0, g=0, b=0;
   for (j=y1; j< y2 ; j++ ) {
       for (i=x1; i< x2 ; i++ ) {
          c = (char*) buf + j * ( w[0] + 1) + i + 4;
          if (*c < 255) {
             r+=pal[*c][0];
             g+=pal[*c][1];
             b+=pal[*c][2];
             pixnum ++;
          }
      } /* endfor */
   } /* endfor */

   if (pixnum) {
      int r1 = r / pixnum;
      int g1 = g / pixnum;
      int b1 = b / pixnum;
   
   
      int diff = 0xFFFFFFF;
      int actdif;

   
      for (i=0;i<256 ;i++ ) {
         actdif = sqr( pal[i][0] - r1 ) + sqr( pal[i][1] - g1 ) + sqr( pal[i][2] - b1 );
         if (actdif < diff) {
            diff = actdif;
            *pix1 = i;
         }
      } 
   
/*
      int sml = ( r1 >> 2) + (( g1 >> 2) << 6) + (( b1 >> 2) << 12);
      *pix1 = truecolor2pal_table[sml];
*/
   
   } else {
      *pix1 = 255;
   }
}



pquickview generateaveragecol ( pwterraintype bdn )
{
   pquickview qv;
   qv = new ( tquickview );
   for ( int dir = 0; dir < sidenum; dir++ ) 
      if ( bdn->picture[dir] ) {

         char* c = (char*) &qv->dir[dir].p1 ;
         int dx,dy;
      
         for ( int i=1; i<6 ;i+=2 ) {
            dx = fieldxsize / i;
            dy = fieldysize / i;
            for ( int k=0;k<i ; k++) {
               for ( int j=0 ; j<i ; j++) {
                   generateaveragecolprt ( k * fieldxsize / i, j * fieldysize / i, (k+1) * fieldxsize / i, (j+1) * fieldysize / i, bdn->picture[dir],  c );
                   c++;
               } /* endfor */
            } /* endfor */
         } /* endfor */
      }

   return qv;
}


pterraintype      loadterraintype(char *       name)
{
   tnfilestream stream ( name, 1 ); 
   return loadterraintype ( &stream );
}

pterraintype      loadterraintype( pnstream stream )
{ 
   int version;
   stream->readdata2 ( version );
   if ( version == terrain_version ) {

      pwterraintype pgbt; 

      pterraintype bbt = new (tterraintype);
   
      stream->readdata2 ( *bbt );
      stream->readpchar( &bbt->name );
   
      for ( int i=0; i<cwettertypennum ;i++ ) {
         if (bbt->weather[i] ) {
            bbt->weather[i] = new ( twterraintype );
            pgbt = bbt->weather[i];
            stream->readdata2 ( *pgbt );
   
            #ifndef converter
            char mmcount = cmovemalitypenum;
   
            if (mmcount < pgbt->movemaluscount )
               mmcount = pgbt->movemaluscount;
            #else
            char mmcount = pgbt->movemaluscount ;
            #endif
   
            bbt->weather[i]->movemalus = new ( char[mmcount ]);
   
            int j;
            for (j=0; j< mmcount ; j++ ) {
               if (j < pgbt->movemaluscount)
                  stream->readdata ( pgbt->movemalus+j, 1);
               else
                  pgbt->movemalus[j] = pgbt->movemalus[0];
   
               if ( pgbt->movemalus[j] == 0) {
                  if (j == 0)
                     pgbt->movemalus[j] = minmalq;
                  else
                     pgbt->movemalus[j] = pgbt->movemalus[0];
               }
            }
            pgbt->movemaluscount = mmcount;
   
   
            for ( j=0; j<8 ;j++ ) {
               if ( pgbt->picture[j] ) {
                 #ifdef HEXAGON
                  if ( pgbt->bi_picture[j] == -1 ) {
                 #endif
                     pgbt->picture[j] = asc_malloc ( fieldsize );
                     stream->readdata ( pgbt->picture[j], fieldsize );
      
                    #ifndef HEXAGON
                     #ifdef sgmain
                      asc_free ( pgbt->picture[j] ) ;
                      pgbt->picture[j] = NULL;
                     #endif
                     if ( pgbt->direcpict[j] ) {
                        pgbt->direcpict[j] = asc_malloc ( fielddirecpictsize );
                        stream->readdata ( pgbt->direcpict[j], fielddirecpictsize );
                     }
                    #endif
                  #ifdef HEXAGON
                   } else {
                      loadbi3pict_double ( pgbt->bi_picture[j], &pgbt->picture[j], gameoptions.bi3.interpolate.terrain );
                   }
                  #endif
               } 
               #ifndef HEXAGON
                 else 
                  if (j == 1) {
   
                     pgbt->picture[j]   = asc_malloc( fieldsize );
                     pgbt->direcpict[j] = asc_malloc ( fielddirecpictsize );
   
                     tvirtualdisplay vd ( 100, 100 );
   
                     memset ( (void*) agmp->linearaddress, 255, agmp->resolutionx * agmp->resolutiony );
                     putrotspriteimage90 ( 10, 10, pgbt->picture[0] , 0);
                     putrotspriteimage90 ( 11, 10, pgbt->picture[0] , 0);
   
                     
                     getimage(10,10, 10 + fieldxsize-1, 10 + fieldysize-1, pgbt->picture[j] );
   
   
                     char *b = (char*) pgbt->direcpict[j];
   
                     for (int t = 1; t < 20; t++) {
                        for (int u = 20-t; u < 20+t; u++) {
                           *b = getpixel(  10 + u, 9 + t);
                           b++;
                        }
                     }
                     
                     for (t = 20; t > 0; t-- ) {
                        for (int u =20-t; u<= 19 + t; u++) {
                           *b = getpixel(  10 + u, 10 + 39 - t );
                           b++;
                        }
                     }
   
                  } else
                     if (j >= 2 && j <= 3) {
         
                        char *b1, *b2;
                        int k;
         
                        pgbt->picture[j] = asc_malloc ( fieldsize );
                        b1 = (char*) pgbt->picture[j-2];
                        b2 = (char*) pgbt->picture[j];
                        for (k=0; k<4; k++,b1++,b2++)
                           *b2 = *b1;
         
                        b2 = (char*) pgbt->picture[j] + fieldsize - 1;
         
                        for (k=4; k<fieldsize; k++) {
                           *b2 = *b1;
                           b1++;
                           b2--;
                        }
         
                        pgbt->direcpict[j] = asc_malloc ( fielddirecpictsize );
                        b1 = (char*) pgbt->direcpict[j-2];
                        b2 = (char*) pgbt->direcpict[j] + fielddirecpictsize - 1;
         
                        for (k=0; k<fielddirecpictsize; k++) {
                           *b2 = *b1;
                           b1++;
                           b2--;
                        }
         
                     }  else
                          if ( j <= 7 ) {
                             pgbt->picture[j]   = asc_malloc ( fieldsize );
                             pgbt->direcpict[j] = asc_malloc ( fielddirecpictsize );
                             flippict( pgbt->picture[j-4], pgbt->picture[j] );
                             generatedirecpict  ( pgbt->picture[j] , pgbt->direcpict[j] );
                          }
               #endif
   
            } /* endfor */
   
            pgbt->terraintype = bbt;
            if ( pgbt->quickview ) {
               pgbt->quickview = new ( tquickview );
               stream->readdata ( pgbt->quickview, sizeof ( *pgbt->quickview ));
            } 
            #ifndef converter
            else
               pgbt->quickview = generateaveragecol ( pgbt );
            #endif
         } else 
            bbt->weather[i] = NULL;
   
      } /* endfor */
   
      return bbt; 
   } else
      return NULL;
} 


void writeterrain ( pterraintype bdt, pnstream stream )
{
   stream->writedata2 ( terrain_version );
   stream->writedata2 ( *bdt );
   stream->writepchar( bdt->name );
   for (int i=0;i<cwettertypennum ;i++ ) {
     if ( bdt->weather[i] ) {
        stream->writedata ( ( char*) bdt->weather[i], sizeof ( *(bdt->weather[i]) ));
        stream->writedata ( bdt->weather[i]->movemalus, bdt->weather[i]->movemaluscount );
        for ( int j = 0; j < 8; j++ ) {
           if ( bdt->weather[i]->picture[j] 
           #ifdef HEXAGON
           && bdt->weather[i]->bi_picture[j] == -1 
           #endif
           ) {
              stream->writedata ( ( char*) bdt->weather[i]->picture[j], fieldsize );
             #ifndef HEXAGON
              if ( bdt->weather[i]->direcpict[j] )
                 stream->writedata ( ( char*) bdt->weather[i]->direcpict[j], fielddirecpictsize );
             #endif
           }
        }
        if ( bdt->weather[i]->quickview )
           stream->writedata ( ( char*) bdt->weather[i]->quickview, sizeof ( *bdt->weather[i]->quickview ));
     }
   }
}



pobjecttype streetobjectcontainer = NULL;
pobjecttype pathobject = NULL;
pobjecttype railroadobject = NULL;
pobjecttype runwayobject = NULL;
pobjecttype eisbrecherobject = NULL;
pobjecttype fahrspurobject = NULL;



pobjecttype   loadobjecttype(char *       name)
{
   tnfilestream stream ( name, 1 );
   return loadobjecttype ( &stream );
/*
   pobjecttype fztn = loadobjecttype ( &stream );
   for ( int i = 0 ; i < cmovemalitypenum; i++) {
       if ( fztn->movemalus_abs[i] > 0  && fztn->movemalus_abs[i] < 10 )
          fprintf(stderr, "The object %s has invalid movemali\n", name );
   }
   return fztn;
*/
}



pobjecttype   loadobjecttype( pnstream stream )
{ 
  int j;
  int version;
  
 stream->readdata2 ( version );
   if ( version == object_version ) {

      pobjecttype fztn = new tobjecttype;
   
      stream->readdata2( *fztn ); 
   
      int copycount = 0;

      int w;
      for ( int ww = 0; ww < cwettertypennum; ww++ )
         if ( fztn->weather & ( 1 << ww )) 
            if ( fztn->pictnum ) {
               fztn->picture[ww] = new thexpic[fztn->pictnum];
               for ( int n = 0; n < fztn->pictnum; n++ ) {
                  stream->readdata2 ( fztn->picture[ww][n] );
                  if ( fztn->picture[ww][n].bi3pic != -1 ) 
                     loadbi3pict_double ( fztn->picture[ww][n].bi3pic, &fztn->picture[ww][n].picture, gameoptions.bi3.interpolate.objects, 0 );
                  else
                     stream->readrlepict ( &fztn->picture[ww][n].picture, false, &w);

                  if ( fztn->picture[ww][n].flip == 1 ) {
                     void* buf = new char [ imagesize ( 0, 0, fieldxsize, fieldysize ) ];
                     flippict ( fztn->picture[ww][n].picture, buf , 1 );
                     asc_free ( fztn->picture[ww][n].picture );
                     fztn->picture[ww][n].picture = buf;
                     copycount++;
                  }
      
                  if ( fztn->picture[ww][n].flip == 2 ) {
                     void* buf = new char [ imagesize ( 0, 0, fieldxsize, fieldysize ) ];
                     flippict ( fztn->picture[ww][n].picture, buf , 2 );
                     asc_free ( fztn->picture[ww][n].picture );
                     fztn->picture[ww][n].picture = buf;
                     copycount++;
                  }
      
                  if ( fztn->picture[ww][n].flip == 3 ) {
                     void* buf = new char [ imagesize ( 0, 0, fieldxsize, fieldysize ) ];
                     flippict ( fztn->picture[ww][n].picture, buf , 2 );
                     flippict ( buf, fztn->picture[ww][n].picture, 1 );
                     delete[] buf;
                     copycount++;
                  }

                  if ( fztn->picture[ww][n].bi3pic == -1 ) 
                     fztn->picture[ww][n].flip = 0;
               }
            }

      if ( copycount == 0 ) 
         for ( int ww = 0; ww < cwettertypennum; ww++ )
            if ( fztn->weather & ( 1 << ww )) 
               if ( fztn->pictnum ) 
                  for ( int n = 0; n < fztn->pictnum; n++ ) 
                     if ( fztn->picture[ww][n].bi3pic != -1 ) {
                        asc_free ( fztn->picture[ww][n].picture );
                        loadbi3pict_double ( fztn->picture[ww][n].bi3pic, &fztn->picture[ww][n].picture, gameoptions.bi3.interpolate.objects );
                     }

     /*
      #ifndef HEXAGON
            if ( fztn->pictnum ) {
               typedef void* pvoid;
               fztn->picture = new pvoid[fztn->pictnum];
               for ( int n = 0; n < fztn->pictnum; n++ )
                  stream->readrlepict ( &fztn->picture[n], false, &w);
            }
      #endif
     */

       #ifndef converter
       char mmcount = cmovemalitypenum;
   
       if (mmcount < fztn->movemalus_plus_count )
          mmcount = fztn->movemalus_plus_count;
       #else
       char mmcount = fztn->movemalus_plus_count ;
       #endif
   
       fztn->movemalus_plus = new char[ mmcount ] ;
   
       for (j=0; j< mmcount ; j++ ) {
          if (j < fztn->movemalus_plus_count )
             stream->readdata ( fztn->movemalus_plus + j, 1);
          else
             if( j > 0 )
               fztn->movemalus_plus[j] = fztn->movemalus_plus[0];
             else
               fztn->movemalus_plus[j] = 0;
   
       }
       fztn->movemalus_plus_count = mmcount;
   
   
   
       #ifndef converter
       mmcount = cmovemalitypenum;
   
       if (mmcount < fztn->movemalus_abs_count )
          mmcount = fztn->movemalus_abs_count;
       #else
       mmcount = fztn->movemalus_abs_count ;
       #endif
   
       fztn->movemalus_abs = new char[ mmcount ] ;
   
       for (j=0; j< mmcount ; j++ ) {
          if (j < fztn->movemalus_abs_count )
             stream->readdata ( fztn->movemalus_abs + j, 1);
          else
             if( j > 0 )
                fztn->movemalus_abs[j] = fztn->movemalus_abs[0];
             else
                fztn->movemalus_abs[j] = 0;
   
       }
       fztn->movemalus_abs_count = mmcount;
   
       if ( fztn->name )
          stream->readpchar ( &fztn->name );


      if ( fztn->dirlistnum ) {
         fztn->dirlist = new int[ fztn->dirlistnum ];
         stream->readdata ( fztn->dirlist, fztn->dirlistnum* 4 );
      } 


      if ( fztn->objectslinkablenum ) {
         fztn->objectslinkableid = new int[ fztn->objectslinkablenum ];
         stream->readdata ( fztn->objectslinkableid, fztn->objectslinkablenum* 4 );
      } 

      #ifdef HEXAGON

       if ( fztn->id == 9 )
          pathobject = fztn;

       if ( fztn->id == 1 )
          streetobjectcontainer = fztn;
   
       if ( fztn->id == 2 )
          railroadobject = fztn;
   
       if ( fztn->id == 6 )
          eisbrecherobject = fztn;
   
       if ( fztn->id == 7 )
          fahrspurobject = fztn;
      #else
       if ( fztn->id == 1 )
          streetobjectcontainer = fztn;
   
       if ( fztn->id == 2 )
          railroadobject = fztn;
   
       if ( fztn->id == 6 )
          eisbrecherobject = fztn;
   
       if ( fztn->id == 7 )
          fahrspurobject = fztn;
      #endif
      #ifndef converter
       fztn->buildicon = generate_object_gui_build_icon ( fztn, 0 );
       fztn->removeicon = generate_object_gui_build_icon ( fztn, 1 );
      #else
       fztn->buildicon = NULL;
       fztn->removeicon = NULL;
      #endif
   
       return fztn;
   } else
       return NULL;
} 

void writeobject ( pobjecttype object, pnstream stream, int compressed )
{
    stream->writedata2 ( object_version ); 
    stream->writedata2 ( *object );
 
    #ifdef HEXAGON
    for ( int ww = 0; ww < cwettertypennum; ww++ )
       if ( object->weather & ( 1 << ww ))
          for ( int l = 0; l < object->pictnum; l++ ) {
             stream->writedata2 ( object->picture[ww][l] );
             if ( compressed ) {
                if ( object->picture[ww][l].bi3pic == -1 )
                   stream->writerlepict( object->picture[ww][l].picture );
             } else
                 if ( object->picture[ww][l].bi3pic == -1 )
                    stream->writedata( object->picture[ww][l].picture, getpicsize2 ( object->picture[ww][l].picture) );
          }
   #else
    if ( object->pictnum ) {     

       if ( compressed ) {
          for ( int i = 0; i < object->pictnum; i++ )
             stream->writerlepict( object->picture[i] );
       } else                                     
          for ( int i = 0; i < object->pictnum; i++ )
             stream->writedata( object->picture[i], getpicsize2 ( object->picture[i] ));
    }
   #endif

    if ( object->movemalus_plus_count ) 
       stream->writedata ( object->movemalus_plus, object->movemalus_plus_count );

    if ( object->movemalus_abs_count ) 
       stream->writedata ( object->movemalus_abs, object->movemalus_abs_count );

    if ( object->name ) 
       stream->writepchar ( object->name );

    if ( object->dirlistnum ) 
       stream->writedata ( object->dirlist, object->dirlistnum* 4 );

    if ( object->objectslinkablenum ) 
       stream->writedata ( object->objectslinkable, object->objectslinkablenum* 4 );


}



void loadpalette ( void )
{
   tnfilestream stream ("palette.pal",1); 
   stream.readdata( & pal, sizeof(pal)); 
   colormixbuf = (pmixbuf) new char [ sizeof ( tmixbuf ) ];
   stream.readdata( colormixbuf,  sizeof ( *colormixbuf ));

   stream.readdata( &xlattables.a.dark05, sizeof ( xlattables.a.dark05 ));
   xlattables.a.dark05[255] = 255;

   stream.readdata( &xlattables.a.dark1,  sizeof ( xlattables.a.dark1 ));
   xlattables.a.dark1[255] = 255;

   stream.readdata( &xlattables.a.dark2,  sizeof ( xlattables.a.dark2 ));
   xlattables.a.dark2[255] = 255;

   stream.readdata( &xlattables.a.dark3,  sizeof ( xlattables.a.dark3 ));
   xlattables.a.dark3[255] = 255;

   stream.readdata( &xlattables.light,  sizeof ( xlattables.light ));
   xlattables.light[255] = 255;

   stream.readdata( &truecolor2pal_table,  sizeof ( truecolor2pal_table ));
   #ifdef HEXAGON
   stream.readdata( &bi2asc_color_translation_table,  sizeof ( bi2asc_color_translation_table ));
   #endif
   asc_paletteloaded = 1;
}



t_carefor_containerstream :: t_carefor_containerstream ( void ) 
{ 
   opencontainer ( "*.con" );
}


bool checkDirectoryExistance ( const char* path )
{
   char tmp[10000];
   constructFileName( tmp, 0, path, NULL );

   int existence = 0;

   DIR *dirp = opendir( tmp );
   if( dirp ) {
      if ( readdir( dirp ) )
         existence = 1;
      else
         existence = 0;

      closedir( dirp );
   }

   if ( !existence ) {
      int res = mkdir ( tmp, 0700 );
      if ( res ) {
         fprintf(stderr, "could neither access nor create work directory %s\n", tmp );
         return false;
      }
   }

   return true;
}


// CLoadableGameOptions* loadableGameOptions = NULL;

char* configFileNameUsed = NULL;
char* configFileNameToWrite = NULL;

char* getConfigFileName ( char* buffer )
{
   if ( buffer ) {
      if ( configFileNameUsed )
         strcpy ( buffer, configFileNameUsed );
      else
         strcpy ( buffer, "-none-" );
   }
   return buffer;
}


int readgameoptions ( const char* filename )
{

   const char* fn;
   if ( filename )
      fn = filename;
   else
      if ( getenv ( asc_EnvironmentName )) 
         fn = getenv ( asc_EnvironmentName );
      else
         fn = asc_configurationfile;

   char completeFileName[10000];
   constructFileName ( completeFileName, -3, NULL, fn );

   configFileNameToWrite = strdup ( completeFileName );

   if ( exist ( completeFileName )) {
      configFileNameUsed = strdup ( completeFileName );

      CLoadableGameOptions* loadable = new CLoadableGameOptions (&gameoptions);
      std::ifstream is( completeFileName );
      loadable->Load(is);	
   } else {
      gameoptions.changed = 1; // to generate a configuration file
      if ( exist ( "sg.cfg" ) ) {
         tnfilestream stream ( "sg.cfg", 1);
         int version = stream.readInt ( );
         if ( version == 102 ) {
            gameoptions.fastmove = stream.readInt();
            gameoptions.visibility_calc_algo = stream.readInt();
            gameoptions.movespeed = stream.readInt();
            gameoptions.endturnquestion = stream.readInt();
            gameoptions.smallmapactive = stream.readInt();
            gameoptions.units_gray_after_move = stream.readInt();
            gameoptions.mapzoom = stream.readInt();
            gameoptions.mapzoomeditor = stream.readInt();
            gameoptions.startupcount = stream.readInt();
            gameoptions.dontMarkFieldsNotAccessible_movement = stream.readInt();
            gameoptions.attackspeed1 = stream.readInt();
            gameoptions.attackspeed2 = stream.readInt();
            gameoptions.attackspeed3 = stream.readInt();
            gameoptions.disablesound = stream.readInt();
            for ( int i = 0; i < 9; i++ )
               stream.readInt();  // dummy

            gameoptions.mouse.scrollbutton = stream.readInt();
            gameoptions.mouse.fieldmarkbutton = stream.readInt();
            gameoptions.mouse.smallguibutton = stream.readInt();
            gameoptions.mouse.largeguibutton = stream.readInt();
            gameoptions.mouse.smalliconundermouse = stream.readInt();
            gameoptions.mouse.centerbutton = stream.readInt();
            gameoptions.mouse.unitweaponinfo = stream.readInt();
            gameoptions.mouse.dragndropmovement = stream.readInt();
            for ( int j = 0; j < 7; j++ )
               stream.readInt();

            gameoptions.container.autoproduceammunition = stream.readInt();
            gameoptions.container.filleverything = stream.readInt();
            gameoptions.container.emptyeverything = stream.readInt();
            for ( int k = 0; k < 10; k++ )
               stream.readInt();

            gameoptions.onlinehelptime = stream.readInt();
            gameoptions.smallguiiconopenaftermove = stream.readInt();
            gameoptions.defaultpassword = stream.readInt();
            gameoptions.replayspeed = stream.readInt();
            int bi3dir = stream.readInt();
            gameoptions.bi3.interpolate.terrain = stream.readInt();
            gameoptions.bi3.interpolate.units = stream.readInt();
            gameoptions.bi3.interpolate.objects = stream.readInt();
            gameoptions.bi3.interpolate.buildings = stream.readInt();

            if ( bi3dir ) {
               char* tmp;
               stream.readpchar ( &tmp );
               gameoptions.bi3.dir.setName( tmp );
               delete[] tmp;
            }

         }
      }
      configFileNameUsed = strdup ( "-none- ; default values used" );
   }
   #ifdef sgmain
   if ( gameoptions.startupcount < 10 ) {
      gameoptions.startupcount++;
      gameoptions.changed = 1;
   }
   #endif

   checkDirectoryExistance ( gameoptions.searchPath[0].getName() );

   return 0;
}

int writegameoptions ( const char* filename )
{
   if ( gameoptions.changed && configFileNameToWrite ) {
      CLoadableGameOptions* loadable = new CLoadableGameOptions (&gameoptions);
      std::ofstream os( configFileNameToWrite );
      loadable->Save(os);
   }
   return 0;
}

void checkFileLoadability ( const char* filename )
{
   try {
      tnfilestream strm ( filename, 1 );
      char a = strm.readChar();
   }
   catch ( tfileerror err ) {
      char temp[10000];
      temp[0] = 0;
      for ( int i = 0; i < 5; i++ )
         if ( gameoptions.searchPath[i].getName() ) {
            strcat ( temp, "  " );
            strcat ( temp, gameoptions.searchPath[i].getName() );
            strcat ( temp, "\n" );
         }

      char temp2[1000];
      displaymessage ( "Unable to access %s\n"
                       "Make sure the data file 'main.con' is in one of the search paths specified in your config file !\n"
                       "The configuration file that is used is: %s \n"
                       "These pathes are being searched:\n%s\n"
                       "If you don't have a file 'main.con' , get and install the data package from www.asc-hq.org\n",
                       2, filename, getConfigFileName(temp2), temp );
   }
}

void initFileIO ( const char* configFileName )
{
   readgameoptions( configFileName );

   for ( int i = 0; i < 5; i++ )
      if ( gameoptions.searchPath[i].getName() ) {
         if ( verbosity > 2 )
            printf("adding search patch %s\n", gameoptions.searchPath[i].getName() );
         addSearchPath ( gameoptions.searchPath[i].getName() );
      }
   try {
     opencontainer ( "*.con" );
   }
   catch ( tfileerror err ) {
      displaymessage ( "a fatal IO error occured while mounting the container files \n", 2 );
   }
   catch ( ... ) {
      displaymessage ( "loading of game failed during pre graphic initializing", 2 );
   }

   checkFileLoadability ( "palette.pal" );
}