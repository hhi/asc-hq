/*! \file palette.h
    \brief The color palette and various color translation tables
*/

//     $Id: palette.h,v 1.10 2007-07-08 15:25:21 mbickel Exp $
//
//     $Log: not supported by cvs2svn $
//     Revision 1.9  2007/04/13 16:15:54  mbickel
//      Merged ASC2 branch
//
//     Revision 1.7.2.1  2004/10/11 18:14:04  mbickel
//      Converted units to use new graphic system
//      Wrote new blitter
//
//     Revision 1.7  2001/11/05 21:10:42  mbickel
//      Updated palette code
//
//     Revision 1.6  2001/09/20 15:36:09  mbickel
//      New object displaying mode
//
//     Revision 1.5  2001/01/28 20:42:14  mbickel
//      Introduced a new string class, ASCString, which should replace all
//        char* and std::string in the long term
//      Split loadbi3.cpp into 3 different files (graphicselector, graphicset)
//
//     Revision 1.4  2001/01/28 14:04:15  mbickel
//      Some restructuring, documentation and cleanup
//      The resource network functions are now it their own files, the dashboard
//       as well
//      Updated the TODO list
//
//     Revision 1.3  1999/12/28 21:03:15  mbickel
//      Continued Linux port
//      Added KDevelop project files
//
//     Revision 1.2  1999/11/16 03:42:20  tmwilson
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

#ifndef palette_h_included
 #define palette_h_included

#include <SDL_stdinc.h>

#pragma pack(1)

typedef Uint8 tmixbuf[256][256];
typedef tmixbuf* pmixbuf;
typedef Uint8 tpixelxlattable[256];
typedef tpixelxlattable* ppixelxlattable; 
typedef Uint8 dacpalette256[256][3];
typedef Uint8 dacpalettefst[256][4];


   struct txlattables {
             tpixelxlattable nochange;
             union {
                struct {
                   tpixelxlattable dark05;
                   tpixelxlattable dark1;
                   tpixelxlattable dark2;
                   tpixelxlattable dark3;
                   tpixelxlattable light;
                   tpixelxlattable light2;
                   tpixelxlattable light3;
                   tpixelxlattable light4;
                }a;
                tpixelxlattable xl[8];
             };
          };

   extern txlattables xlattables;

/*
extern tpixelxlattable nochange;
extern tpixelxlattable dark1;
extern tpixelxlattable dark2;
extern tpixelxlattable light;
*/

// tables storing the color that results when two palette colors are mixed
extern tmixbuf *colormixbuf;
extern Uint8* colormixbufchar;


extern dacpalette256 pal;

extern bool asc_paletteloaded;
extern ppixelxlattable xlatpictgraytable;  
// extern tpixelxlattable bi2asc_color_translation_table;

#pragma pack()

#endif
