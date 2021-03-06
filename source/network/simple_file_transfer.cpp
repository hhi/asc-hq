/*! \file network.cpp
    \brief Code for moving a multiplayer game data from one computer to another.
 
    The only method that is currently implemented is writing the data to a file
    and telling the user to send this file by email :-)
    But the interface for real networking is there...
*/
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

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include "../global.h"
#include "../misc.h"
#include "../loaders.h"
#include "../gamemap.h"
#include "../dlg_box.h"
#include "../dialog.h"
#include "simple_file_transfer.h"
#include "../dialogs/fileselector.h"
#include "../gameoptions.h"
#include "../basestrm.h"

void FileTransfer::readChildData ( tnstream& stream )
{
   stream.readInt();
   filename = stream.readString();
}

void FileTransfer::writeChildData ( tnstream& stream ) const
{
   stream.writeInt( 1 );
   stream.writeString( filename );
}

void FileTransfer::setup()
{
   enterfilename();
}


void FileTransfer::setup( const ASCString& filename )
{
   this->filename = filename;
}

bool FileTransfer::enterfilename()
{
   filename = editString( "Enter Filename", filename );
   return true;
}


void FileTransfer::send( const GameMap* map, int lastPlayer, int lastturn )
{
   while ( filename.empty() ) {
      if( !enterfilename() )
         return;
   }

   try {      
      ASCString fname = constructFileName( map, lastPlayer, lastturn );

      tfindfile ff( fname );
      tfindfile::FileInfo fi;
      if ( ff.getnextname( fi ) && fi.directoryLevel == 0 )
         do {
            if (choice_dlg("overwrite " + fname,"~y~es","~n~o") == 2) 
               fname = selectFile( ASCString("*") + tournamentextension, false );
         } while (fname.empty());

      {
         tnfilestream gamefile ( fname, tnstream::writing );
         tnetworkloaders nwl;
         nwl.savenwgame( &gamefile, map );
      }

      int nextPlayer = map->actplayer;
      if ( nextPlayer < 0 )
         nextPlayer = 0;
      
      bool mail = !CGameOptions::Instance()->mailProgram.empty();


      if ( mail )
         if (choiceDialog("Invoke the mail program to send the file?","~m~ail","~j~ust save", "sendpbem") == 2)
            mail = false;


      if ( !mail ) {
         
         ASCString msg = "Data written!\nPlease send " + fname + " to \n" + map->player[nextPlayer].getName();
         if ( !map->player[nextPlayer].email.empty() )
            msg += " (" + map->player[nextPlayer].email + ")";
         infoMessage( msg );
      } else {
         
         ASCString fullFile = ::constructFileName(0, "", fname);
         
         ASCString command = CGameOptions::Instance()->mailProgram;
         command.replaceAll_ci( "$(fullfile)", fullFile);
         command.replaceAll_ci( "$(file)", fname );
         
         ASCString url = fullFile;
         if ( pathdelimitterstring != ASCString("/"))
            url.replaceAll(pathdelimitterstring, "/");
         
         if ( url.find('/') == 0 )
            url = "file://" + url;
         else
            url = "file:///" + url;

         command.replaceAll_ci( "$(url)", url );
         
         if ( map->player[nextPlayer].email.empty() )
            command.replaceAll_ci( "$(to)", "unknown");
         else
            command.replaceAll_ci( "$(to)", map->player[nextPlayer].email );
         
         int sv = -1;
         for ( int i = 0; i < map->getPlayerCount(); ++i)
            if ( map->getPlayer(i).stat == Player::supervisor ) {
               sv = i;
               break;
            }
           
         if ( sv != -1 && sv != nextPlayer )
            command.replaceAll_ci( "$(sv)", map->player[sv].email );
         else
            command.replaceAll_ci( "$(sv)", "" );
         
         StatusMessageWindowHolder smw = MessagingHub::Instance().infoMessageWindow( "Executing external mailer:\n" + command );
         
#ifdef _WIN32_
         // Windows sucks!
         ASCString realCommand = "\"" + command + "\"";
#else
         ASCString& realCommand = command;
#endif

         int res = system(realCommand.c_str());
         if (res != 0 ) {
            smw.close();
            errorMessage("Program failed with exit code " + ASCString::toString(res) + "\nCommand was:\n" + command);
         } else {
            smw.close();
            infoMessage( "Mail submitted successfully" );
         }
      }
   } catch ( tfileerror ) {
      errorMessage ( "error writing file " + filename );
   }
}


GameMap* FileTransfer::receive()
{
   return loadPBEMFile( filename );
}   

GameMap* FileTransfer::loadPBEMFile( const ASCString& filename )
{
   GameMap* map = NULL;
   try {      
      tnfilestream gamefile ( filename, tnstream::reading );
      tnetworkloaders nwl;
      map = nwl.loadnwgame( &gamefile );
      if ( map->actplayer < 0 )
         throw ASCmsgException("this map has not been properly started");
   } 
   catch ( tinvalidversion iv ) {
      throw iv;
   }
   catch ( tfileerror ) {
      errorMessage ( filename + " is not a legal email game" );
      return NULL;
   }
   return map;
}

ASCString FileTransfer::constructFileName( const GameMap* actmap, int lastPlayer, int lastturn ) const
{
   ASCString s = filename;
   while ( s.find( "$p") != ASCString::npos ) {
      if ( lastPlayer >= 0 )
         s.replace( s.find( "$p"), 2, 1, 'A' + lastPlayer );
      else
         s.replace( s.find( "$p"), 2, "SV" );
   }

   while ( s.find( "$t") != ASCString::npos ) {
      if ( lastturn >= 0 )
         s.replace( s.find( "$t"), 2, ASCString::toString( lastturn ) );
      else
         s.replace( s.find( "$t"), 2, ASCString::toString ( actmap->time.turn() ) );
   }

   if ( !s.endswith(tournamentextension ))
      s += tournamentextension;
   return s;
}


namespace {
   const bool r1 = networkTransferMechanismFactory::Instance().registerClass( FileTransfer::mechanismID(), ObjectCreator<GameTransferMechanism, FileTransfer> );
}

