/***************************************************************************
                         graphics.cpp  -  description
                             -------------------
    begin                : Sat Dec 18 1999
    copyright            : (C) 1999 by Martin Bickel
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

//     $Id: graphics.cpp,v 1.20 2002-02-21 17:06:54 mbickel Exp $
//
//     $Log: not supported by cvs2svn $
//     Revision 1.19  2002/02/14 20:58:14  mbickel
//      Started integration of paragui
//
//     Revision 1.18  2001/10/16 15:33:03  mbickel
//      Added icons to data
//      ASC displays icons
//      Fixed crash when building icons
//      Removed assembled datafiles
//
//     Revision 1.17  2001/08/02 15:33:02  mbickel
//      Completed text based file formats
//
//     Revision 1.16  2000/10/18 12:40:48  mbickel
//      Rewrite event handling for windows
//
//     Revision 1.15  2000/10/16 14:34:13  mbickel
//      Win32 port is now running fine.
//      Removed MSVC project files and put them into a zip file in
//        asc/source/win32/msvc/
//
//     Revision 1.14  2000/10/14 14:16:11  mbickel
//      Cleaned up includes
//      Added mapeditor to win32 watcom project
//
//     Revision 1.13  2000/10/11 14:26:57  mbickel
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
//     Revision 1.12  2000/08/21 17:51:04  mbickel
//      Fixed: crash when unit reaching max experience
//      Fixed: crash when displaying research image
//      Fixed: crash when events referenced a unit that has been shot down
//      Fixed: screenshot being written to wrong directory
//
//     Revision 1.11  2000/08/12 09:17:42  gulliver
//     *** empty log message ***
//
//     Revision 1.10  2000/06/01 15:27:47  mbickel
//      Some changes for the upcoming Win32 version of ASC
//      Fixed error at startup: unable to load smalaril.fnt
//
//     Revision 1.9  2000/02/08 08:26:54  steb
//     Changed SDL.h to SDL/SDL.h to make installation on the average users system
//     easier.
//
//     Revision 1.8  2000/01/31 16:08:40  mbickel
//      Fixed crash in line
//      Improved error handling in replays
//
//     Revision 1.7  2000/01/07 13:20:07  mbickel
//      DGA fullscreen mode now working
//
//     Revision 1.6  2000/01/06 14:11:22  mbickel
//      Fixed a graphic bug in PD and disabled fullscreen mode
//
//     Revision 1.5  2000/01/06 11:19:16  mbickel
//      Worked on the Linux-port again...
//
//     Revision 1.4  2000/01/04 19:43:55  mbickel
//      Continued Linux port
//
//     Revision 1.3  2000/01/02 19:47:09  mbickel
//      Continued Linux port
//      Fixed crash at program exit
//
//     Revision 1.2  2000/01/01 19:04:20  mbickel
//     /tmp/cvsVhJ4Z3
//
//     Revision 1.1  1999/12/28 21:03:31  mbickel
//      Continued Linux port
//      Added KDevelop project files
//


#include <stdlib.h>
#include "graphics.h"
#include "../basegfx.h"
#include "../global.h"
#include "../ascstring.h"
#include "../errors.h"
#include sdlheader



SDL_Surface *screen = NULL;
int fullscreen = 1;

int reinitgraphics(int modenum)
{
  return 1;
}

int isfullscreen ( void )
{
   if ( !screen )
      return 0;
   else
      return screen->flags & SDL_FULLSCREEN;
}

void setWindowCaption ( const char* s )
{
   SDL_WM_SetCaption ( s, NULL );
}

int initgraphics ( int x, int y, int depth, SDLmm::Surface* icon )
{
  if ( SDL_Init(SDL_INIT_VIDEO  ) < 0 )  // | SDL_INIT_NOPARACHUTE
     fatalError ( ASCString("Couldn't initialize SDL: ") + SDL_GetError());


  setWindowCaption ( "Advanced Strategic Command" );

  if ( icon )
     SDL_WM_SetIcon( icon->GetSurface(), NULL );


  /* Initialize the display in a 640x480 8-bit palettized mode */
  int flags = SDL_SWSURFACE;
  if ( fullscreen )
     flags |= SDL_FULLSCREEN;

  SDL_Surface* screen = SDL_SetVideoMode(x, y, depth, flags ); // | SDL_FULLSCREEN
  if ( !screen )
     fatalError ( "Couldn't set %dx%dx%d video mode: %s\n", x,y,depth, SDL_GetError());

  initASCGraphicSubsystem ( screen, icon );

  return 1;
}

void initASCGraphicSubsystem ( SDL_Surface* _screen, SDLmm::Surface* icon )
{
  screen = _screen;
  agmp->resolutionx = screen->w;
  agmp->resolutiony = screen->h;
  agmp->windowstatus = 100;
  agmp->scanlinelength = screen->w;
  agmp->scanlinenumber = screen->h;
  agmp->bytesperscanline = screen->w * screen->format->BytesPerPixel;
  agmp->byteperpix = screen->format->BytesPerPixel ;
  agmp->linearaddress = (int) screen->pixels;
  agmp->bitperpix = screen->format->BitsPerPixel;
  agmp->directscreenaccess = 0;

  *hgmp = *agmp;

  graphicinitialized = 1;
}


void  closegraphics ( void )
{
        SDL_FreeSurface ( screen );
        screen = NULL;
}

SDL_Surface* getScreen()
{
   return screen;
}



//*********** Misc ************


int copy2screen( void )
{
  #ifdef _WIN32_  
   SDL_ShowCursor(0);
  #endif 
   SDL_UpdateRect ( screen , 0,0,0,0 );
  #ifdef _WIN32_  
   SDL_ShowCursor(1);
  #endif 
   return 0;
}

int copy2screen( int x1, int y1, int x2, int y2 )
{
  #ifdef _WIN32_  
   SDL_ShowCursor(0);
  #endif 
   if ( x1 == -1 || y1 == -1 || x2 == -1 || y2 == -1 )
      SDL_UpdateRect ( screen , 0,0,0,0 );
   else
      if ( x1 <= x2 && y1 <= y2 )
         SDL_UpdateRect ( screen , x1, y1, x2-x1+1, y2-y1+1 );
      else
         if( x1 > x2 )
            SDL_UpdateRect ( screen , x2, y1, x1-x2+1, y2-y1+1 );
         else
            SDL_UpdateRect ( screen , x1, y2, x2-x1+1, y1-y2+1 );


  #ifdef _WIN32_  
   SDL_ShowCursor(1);
  #endif 
          
   return 0;
}


void setdisplaystart( int x, int y)
{
}

void set_vgapalette256 ( dacpalette256 pal )
{
        SDL_Color spal[256];
        int col;
        for ( int i = 0; i < 256; i++ ) {
           for ( int c = 0; c < 3; c++ ) {
         if ( pal[i][c] == 255 )
            col = activepalette[i][c];
         else {
            col = pal[i][c];
            activepalette[i][c] = col;
         }
         switch ( c ) {
            case 0: spal[i].r = col * 4; break;
            case 1: spal[i].g = col * 4; break;
            case 2: spal[i].b = col * 4; break;
         };
     }
        }       
        SDL_SetColors ( screen, spal, 0, 256 );
}

int dont_use_linear_framebuffer;

