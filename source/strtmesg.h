//     $Id: strtmesg.h,v 1.9 2009-04-18 13:48:38 mbickel Exp $
//
//     $Log: not supported by cvs2svn $
//     Revision 1.8  2007/04/13 16:15:54  mbickel
//      Merged ASC2 branch
//
//     Revision 1.6.2.1  2006/06/03 20:42:23  mbickel
//      Improving cargo dialog
//      General polishing
//
//     Revision 1.6  2003/08/28 18:08:28  mbickel
//      Added version information to Play Time dialog
//
//     Revision 1.5  2002/10/12 17:28:04  mbickel
//      Fixed "enemy unit loaded" bug.
//      Changed map format
//      Improved log messages
//
//     Revision 1.4  2001/07/18 16:05:47  mbickel
//      Fixed: infinitive loop in displaying "player exterminated" msg
//      Fixed: construction of units by units: wrong player
//      Fixed: loading bug of maps with mines
//      Fixed: invalid map parameter
//      Fixed bug in game param edit dialog
//      Fixed: cannot attack after declaring of war
//      New: ffading of sounds
//
//     Revision 1.3  2001/05/17 14:23:20  mbickel
//      Rewrote command line parameters of all programs
//      Made manpages generation optional
//
//     Revision 1.2  1999/11/16 03:42:40  tmwilson
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

#ifndef strtmesgH
#define strtmesgH

#include "ascstring.h"


//! Version: 1.2.3 ; compiled : ...
extern ASCString getVersionAndCompilation();


extern ASCString getstartupmessage (  );
extern ASCString getaboutmessage (  );

extern ASCString kgetstartupmessage (  );
extern ASCString kgetaboutmessage (  );

extern const char* getVersionString (  );
extern const char* getFullVersionString (  );
extern ASCString getVersionString ( int version );

extern int getNumericVersion ( );


#endif
