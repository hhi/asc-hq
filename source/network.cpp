/*! \file network.cpp
    \brief Code for moving a multiplayer game data from one computer to another.
 
    The only method that is currently implemented is writing the data to a file
    and telling the user to send this file by email :-)
    But the interface for real networking is there...
*/


//     $Id: network.cpp,v 1.26 2002-02-21 17:06:51 mbickel Exp $
//
//     $Log: not supported by cvs2svn $
//     Revision 1.25  2001/12/19 17:16:29  mbickel
//      Some include file cleanups
//
//     Revision 1.24  2001/10/02 14:06:28  mbickel
//      Some cleanup and documentation
//      Bi3 import tables now stored in .asctxt files
//      Added ability to choose amoung different BI3 import tables
//      Added map transformation tables
//
//     Revision 1.23  2001/07/15 10:36:25  mbickel
//      Added explanation message in email game save dialog
//
//     Revision 1.22  2001/07/13 19:33:30  mbickel
//      Fixed crashes in the dashboards experience display
//      Fixed inconsistent movement cost calculation (which caused
//            AI and replay warnings)
//      Fixed crash when starting network game from main menu
//
//     Revision 1.21  2001/07/13 14:02:48  mbickel
//      Fixed inconsistency in replay (shareviewchange)
//      Fixed sound initialization problem
//      Speed up of movement
//
//     Revision 1.20  2001/07/11 20:44:37  mbickel
//      Removed some vehicles from the data file.
//      Put all legacy units in into the data/legacy directory
//
//     Revision 1.19  2001/07/09 17:38:52  mbickel
//      New default email filename
//      removed limitation for 8 character long email filenames
//
//     Revision 1.18  2001/02/26 12:35:24  mbickel
//      Some major restructuing:
//       new message containers
//       events don't store pointers to units any more
//       tfield class overhauled
//
//     Revision 1.17  2001/02/18 15:37:16  mbickel
//      Some cleanup and documentation
//      Restructured: vehicle and building classes into separate files
//         tmap, tfield and helper classes into separate file (gamemap.h)
//      basestrm : stream mode now specified by enum instead of int
//
//     Revision 1.16  2001/01/31 14:52:41  mbickel
//      Fixed crashes in BI3 map importing routines
//      Rewrote memory consistency checking
//      Fileselect dialog now uses ASCStrings
//
//     Revision 1.15  2001/01/28 14:04:14  mbickel
//      Some restructuring, documentation and cleanup
//      The resource network functions are now it their own files, the dashboard
//       as well
//      Updated the TODO list
//
//     Revision 1.14  2000/10/18 14:14:15  mbickel
//      Rewrote Event handling; DOS and WIN32 may be currently broken, will be
//       fixed soon.
//
//     Revision 1.13  2000/10/14 14:16:06  mbickel
//      Cleaned up includes
//      Added mapeditor to win32 watcom project
//
//     Revision 1.12  2000/10/14 10:52:52  mbickel
//      Some adjustments for a Win32 port
//
//     Revision 1.11  2000/08/12 12:52:49  mbickel
//      Made DOS-Version compile and run again.
//
//     Revision 1.10  2000/08/12 09:17:32  gulliver
//     *** empty log message ***
//
//     Revision 1.9  2000/08/01 13:50:52  mbickel
//      Chaning the height of airplanes is not affected by wind any more.
//      Fixed: Airplanes could ascend onto buildings
//
//     Revision 1.8  2000/07/29 14:54:39  mbickel
//      plain text configuration file implemented
//
//     Revision 1.7  2000/05/30 18:39:25  mbickel
//      Added support for multiple directories
//      Moved DOS specific files to a separate directory
//
//     Revision 1.6  2000/05/22 15:40:36  mbickel
//      Included patches for Win32 version
//
//     Revision 1.5  2000/01/31 16:34:46  mbickel
//      now standard hotkeys in dialog boxes
//
//     Revision 1.4  1999/12/07 22:13:24  mbickel
//      Fixed various bugs
//      Extended BI3 map import tables
//
//     Revision 1.3  1999/11/18 17:31:17  mbickel
//      Improved BI-map import translation tables
//      Moved macros to substitute Watcom specific routines into global.h
//
//     Revision 1.2  1999/11/16 03:42:13  tmwilson
//      Added CVS keywords to most of the files.
//      Started porting the code to Linux (ifdef'ing the DOS specific stuff)
//      Wrote replacement routines for kbhit/getch for Linux
//      Cleaned up parts of the code that gcc barfed on (char vs unsigned char)
//      Added autoconf/automake capabilities
//      Added files used by 'automake --gnu'
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

#include <stdio.h>                    
#include <stdlib.h>
#include <cstring>

#include "global.h"
#include "misc.h"
#include "typen.h"
#include "network.h"
#include "dlg_box.h"
#include "dialog.h"
#include "events.h"
#include "sgstream.h"
#include "loadpcx.h"
#include "mapdisplay.h"
#include "password_dialog.h"
#include "stack.h"
#include "gamedlg.h"

pbasenetworkconnection firstnetworkconnection = NULL;

tfiletransfernetworkconnection filetransfernetworkconnection;
pbasenetworkconnection defaultnetworkconnection  = &filetransfernetworkconnection;


tbasenetworkconnection::tbasenetworkconnection ( void )
{
   next = firstnetworkconnection;
   firstnetworkconnection = this;
   stream = NULL;
}

tbasenetworkconnection::~tbasenetworkconnection ( )
{
}


tfiletransfernetworkconnection::tfiletransfernetworkconnection ( void )
{
   orgstream = NULL;
   strcpy( suffix, tournamentextension );
}

char* tfiletransfernetworkconnection::getname ( void )                
{
   return "direct file transfer";
}


int   tfiletransfernetworkconnection::getid ( void )                
{
   return 1;  // auch dialogboxen �ndern !
}



tfiletransfernetworkconnection::tsetup::tsetup ( void )
{
   exitpossible = 1;
}

void tfiletransfernetworkconnection::tsetup::init ( void )
{
   status = 0;
   xsize = 400;
   ysize = 300;
   x1 = -1;
   y1 = -1;
   addbutton ( "~f~ilename", 20, starty + 40, xsize - 160, starty + 70, 1 , 0, 1, true );
   addeingabe ( 1, filename, 1, maxfilenamelength );

   addbutton ( "~s~elect",   xsize - 140, starty + 40, xsize - 20, starty + 70, 0 , 1, 2, true );

   if ( exitpossible ) {
      addbutton ( "~O~k", 10, ysize - 40, xsize / 2 - 5, ysize - 10, 0, 1, 3 , true);
      addkey ( 3, ct_enter );
      addbutton ( "e~x~it", xsize / 2 + 5, ysize - 40, xsize - 10, ysize - 10, 0, 1, 4, true );
      addkey ( 4, ct_esc );
   } else {
      addbutton ( "~O~k", 10, ysize - 40, xsize - 10, ysize - 10, 0, 1, 3 , true);
      addkey ( 3, ct_enter );
   }
   if ( !filename[0] && actmap ) {
      ASCString fn = actmap->preferredFileNames.mapname[0];
      if ( fn.find ( "." ) != ASCString::npos )
         fn.erase ( fn.find ( "." ) );

      fn += "-";
      char buf = 'A'+actmap->actplayer;
      fn += buf;
      fn += "-%";
      strcpy ( filename, fn.c_str() );
      //strcpy ( filename, "turnier%");
   }

   buildgraphics();
}

void  tfiletransfernetworkconnection::tsetup::buttonpressed ( int id )
{
   ASCString s1;

   tdialogbox::buttonpressed ( id );
   switch ( id ) {
      case 2:    mousevisible( false ); 
                 fileselectsvga( tournamentextension, s1, true );
                 if ( !s1.empty() ) {
                    strcpy ( filename, s1.c_str() );
                    showbutton ( 1 );
                 }
                 mousevisible( true );
                 break;
                 
      case 3:    status = 2;           
                 break;
   
      case 4:    status = 1;
                 break;
                 
   } /* endswitch */

}


void tfiletransfernetworkconnection::treceivesetup::init ( pnetworkconnectionparameters  d )
{
   tdialogbox::init ();
   dtaptr = d;
   memcpy ( dta , dtaptr, sizeof ( dta ));

   filename = &dta[8];

   int* pi = (int*) dta;
   pi[0] = 1;
   pi[1] = 1;


   title = ttl;
   strcpy ( ttl, "direct file transfer setup for receiving" );
   /*tfiletransfernetworkconnection::*/tsetup::init (  );
}

void tfiletransfernetworkconnection::tsendsetup::init ( pnetworkconnectionparameters  d, int exitposs  )
{
   tdialogbox::init ();
   title = ttl;
   strcpy ( ttl, "direct file transfer setup for sending" );

   dtaptr = d;
   memcpy ( dta , dtaptr, sizeof ( dta ));
   filename = &dta[8];

   int* pi = (int*) dta;
   pi[0] = 1;
   pi[1] = 2;
   exitpossible = exitposs;

   /*tfiletransfernetworkconnection::*/tsetup::init (  );

   activefontsettings.font = schriften.smallarial;
   activefontsettings.justify = lefttext;
   activefontsettings.length = 0;
   showtext2("Please enter the filename into which ASC will", x1+25, y1+130 );
   showtext2("write your game. Send this file to the next player.", x1+25, y1+150 );
   showtext2("You will not be asked for this filename again during", x1+25, y1+170 );
   showtext2("this game, so place the % character somehwere.", x1+25, y1+190 );
   showtext2("The % character will automatically be replaced by", x1+25, y1+210 );
   showtext2("the turn number to allow archiving of the files.", x1+25, y1+230 );
}


void tfiletransfernetworkconnection::tsetup::run ( void )
{
   mousevisible ( true );
   do {
       tdialogbox::run();
   } while ( status == 0 ); /* enddo */
   if ( status == 2 ) 
     memcpy ( dtaptr , dta, sizeof ( dta ));

}


int   tfiletransfernetworkconnection::setupforsending   ( pnetworkconnectionparameters  data, int exitpossible  )
{

   tsendsetup sendsetup;
   sendsetup.init( data, exitpossible );
   sendsetup.run();
   sendsetup.done();
   return sendsetup.status-1;
}

int   tfiletransfernetworkconnection::setupforreceiving ( pnetworkconnectionparameters  data )
{
   treceivesetup receivesetup;
   receivesetup.init( data );
   receivesetup.run();
   receivesetup.done();
   return receivesetup.status-1;
}


void  tfiletransfernetworkconnection::initconnection  ( tnetworkchannel channel )          
{
   chann = channel;
}

int   tfiletransfernetworkconnection::connectionopen  ( void )          
{
   return 0;
}


void  tfiletransfernetworkconnection::closeconnection ( void )          
{

}

void  tfiletransfernetworkconnection::mountfilename ( char* newname, char* oldname )
{
   strcpy ( newname, oldname );

   int p = 0; 
   while ( newname[p] != 0  && newname[p] != '%' )
      p++;

   if ( newname[p] == '%' ) {
      int r = p;
      newname[p] = 0;
      char temp[10];
      itoa ( actmap->time.turn(), temp, 10 );
      while ( strlen ( temp ) + strlen ( newname ) > maxfilenamelength ) {
         p--;
         newname[p] = 0;
      }
      strcat ( newname, temp );
      strcat ( newname, &oldname[r+1] );

      p = strlen ( newname );
      if ( !strchr ( newname, '.' ) ) {
         while ( strlen ( newname ) > maxfilenamelength ) {
            p--;
            newname[p] = 0;
         }
      } else {
         while ( strlen ( newname ) > maxfilenamelength ) {
            p--;
            newname[p] = 0;
         }
      }
   }

   if ( strchr ( newname, '.' ) == NULL )
      strcat ( newname, &suffix[1] );

}


int   tfiletransfernetworkconnection::validateparams ( pnetworkconnectionparameters data, tnetworkchannel chann  )
{
   int* pi = (int*) data;
   if ( pi[0] != getid() )
      return 0;
      
   filename = &(*data)[8];
   if ( !filename[0] )
      return 0;

   if ( chann == TN_SEND ) {
      char tempfilename[200];
      mountfilename ( tempfilename, filename );

      if ( exist( tempfilename ) ) {
         char stempp[100];
         sprintf(stempp, "file %s already exists ! Overwrite ?", tempfilename );
         if (choice_dlg(stempp,"~y~es","~n~o") == 2) 
            return 0;
      }

   } else {
      char temp[20];
      strcpy ( temp, filename );
      if ( strchr ( temp, '.' ) == NULL )
         strcat ( temp, &suffix[1] );
      if ( !exist( temp ) )
         return 0;
   }

   return 1;
}


int   tfiletransfernetworkconnection::transferopen  ( void )          
{
   if ( stream || orgstream )
      return 1;
   else 
      return 0;
}


void  tfiletransfernetworkconnection::inittransfer  ( pnetworkconnectionparameters data )          
{
   if ( stream )
      displaymessage ( "tfiletransfernetworkconnection::inittransfer ( pnetworkconnectionparameters ) \n stream bereits initialisiert !",2);

   if ( orgstream )
      displaymessage ( "tfiletransfernetworkconnection::inittransfer ( pnetworkconnectionparameters ) \n orgstream bereits initialisiert !",2);

   while ( validateparams ( data, chann ) == 0 ) {
      if ( chann == TN_SEND )
        setupforsending ( data, 0 );
      else
        setupforreceiving ( data );
   }

   char tempfilename[200];
   mountfilename ( tempfilename, filename );

   if ( chann == TN_SEND )
      orgstream = new  tnfilestream ( tempfilename, tnstream::writing );
   else
      orgstream = new  tnfilestream ( tempfilename, tnstream::reading );

   stream = orgstream;
}

void  tfiletransfernetworkconnection::closetransfer ( void )          
{
   if ( orgstream ) { 
      delete orgstream;
      orgstream = NULL;
      stream = NULL;
   }
}






pbasenetworkconnection getconnectforid( int id )
{
   pbasenetworkconnection conn = firstnetworkconnection;
   while ( conn && conn->getid() != id  )
      conn = conn->next;

   return conn;
}

void setallnetworkpointers ( pnetwork net )
{
   for (int i = 0; i < 8; i++) {
      if ( net->computer[i].send.transfermethodid )
         net->computer[i].send.transfermethod = getconnectforid ( net->computer[i].send.transfermethodid );
      if ( net->computer[i].receive.transfermethodid )
         net->computer[i].receive.transfermethod = getconnectforid ( net->computer[i].receive.transfermethodid );
   } /* endfor */
}



void networksupervisor ( void )
{
   class tcarefordeletionofmap {
         pmap tmp;
      public:
         tcarefordeletionofmap ()
         {
            tmp= actmap;
            actmap = NULL;
         }
         ~tcarefordeletionofmap (  )
         {
            if ( actmap && (actmap->xsize > 0  ||  actmap->ysize > 0) )
               delete actmap;
            actmap = tmp;
         };
   }
   carefordeletionofmap;


   tlockdispspfld ldsf;

   tnetwork network;
   /*
      int stat;
      do {
         stat = setupnetwork( &network, 1+8 );
         if ( stat == 1 )
            return;

      } while ( (network.computer[0].receive.transfermethod == 0) || (network.computer[0].receive.transfermethodid != network.computer[0].receive.transfermethod->getid()) );
   */
   int stat;
   int go = 0;
   do {
      stat = network.computer[0].receive.transfermethod->setupforreceiving ( &network.computer[0].receive.data );
      if ( stat == 0 )
         return;

      if ( network.computer[0].receive.transfermethod  &&
            network.computer[0].receive.transfermethodid == network.computer[0].receive.transfermethod->getid()  &&
            network.computer[0].receive.transfermethod->validateparams( &network.computer[0].receive.data, TN_RECEIVE ))
         go = 1;
   } while ( !go );

   try {
      displaymessage ( " starting data transfer ",0);

      network.computer[0].receive.transfermethod->initconnection ( TN_RECEIVE );
      network.computer[0].receive.transfermethod->inittransfer ( &network.computer[0].receive.data );

      tnetworkloaders nwl;
      nwl.loadnwgame ( network.computer[0].receive.transfermethod->stream );

      network.computer[0].receive.transfermethod->closetransfer();
      network.computer[0].receive.transfermethod->closeconnection();

      removemessage();
      if ( actmap->network )
         setallnetworkpointers ( actmap->network );
   } /* endtry */

   catch ( tfileerror ) {
      displaymessage ("a file error occured while loading game",1 );
      delete actmap;
      actmap = NULL;
      return;
   } /* endcatch */
   catch ( ASCexception ) {
      displaymessage ("error loading game",1 );
      delete actmap;
      actmap = NULL;
      return;
   } /* endcatch */


   int ok = 0;
   if ( !actmap->supervisorpasswordcrc.empty() ) {
      ok = enterpassword ( actmap->supervisorpasswordcrc );
   } else {
      displaymessage ("no supervisor defined",1 );
      delete actmap;
      actmap = NULL;
      return;
   }

   if ( ok ) {
      npush ( actmap->actplayer );
      actmap->actplayer = -1;
      setupalliances( 1 );
      npop ( actmap->actplayer );

      do {
         stat = setupnetwork( &network, 2+8 );
         if ( stat == 1 ) {
            displaymessage ("no changes were saved",1 );
            delete actmap;
            actmap = NULL;
            return;
         }

      } while ( (network.computer[0].send.transfermethod == 0) || (network.computer[0].send.transfermethodid != network.computer[0].send.transfermethod->getid()) ); /* enddo */

      tnetworkcomputer* compi = &network.computer[ 0 ];

      displaymessage ( " starting data transfer ",0);

      try {
         compi->send.transfermethod->initconnection ( TN_SEND );
         compi->send.transfermethod->inittransfer ( &compi->send.data );

         tnetworkloaders nwl;
         nwl.savenwgame ( compi->send.transfermethod->stream );

         compi->send.transfermethod->closetransfer();
         compi->send.transfermethod->closeconnection();
      } /* endtry */
      catch ( tfileerror ) {
         displaymessage ( "a file error occured while saving file", 1 );
      } /* endcatch */
      catch ( ASCexception ) {
         displaymessage ( "error saving file", 1 );
      } /* endcatch */

      delete actmap;
      actmap = NULL;
      displaymessage ( "data transfer finished",1);

   } else {
      displaymessage ("no supervisor defined or invalid password",1 );
      delete actmap;
      actmap = NULL;
   }
}
