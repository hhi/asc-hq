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

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <map>

#include "../tpascal.inc"
#include "../typen.h"
#include "../sgstream.h"
#include "../misc.h"
#include "../basestrm.h"
#include "../basegfx.h"
#include "../buildingtype.h"
#include "../vehicletype.h"
#include "../graphicset.h"
#include "../graphicselector.h"
#include "../strtmesg.h"
#include "../textfileparser.h"



// including the command line parser, which is generated by genparse
#include "../clparser/bin2txt.cpp"

int main(int argc, char *argv[] )
{
   Cmdline cl ( argc, argv );

   if ( cl.v() ) {
      cout << argv[0] << " " << getVersionString() << endl;
      exit(0);
   }

   verbosity = cl.r();

   initFileIO( cl.c().c_str() );  // passing the filename from the command line options

   if ( cl.next_param() >= argc) {
      cl.usage();
      exit(0);
   }


   try {
      loadpalette();
      loadbi3graphics();

      for ( int i = cl.next_param(); i < argc; i++ ) {
         tfindfile ff ( argv[i] );
         ASCString filename = ff.getnextname();
         while ( !filename.empty() ) {
            if ( patimat ( "*.obl", filename.c_str() )) {
               ObjectType* ot = loadobjecttype ( filename.c_str() );

               PropertyWritingContainer pc ( "ObjectType", ot->fileName + ".asctxt" );
               cout << "Writing file " << pc.getFilename() << "... ";
               ot->runTextIO ( pc );
               pc.run();

               cout << "done \n";

               delete ot;
            }
            if ( patimat ( "*.trr", filename.c_str() )) {
               TerrainType* tt = loadterraintype ( filename.c_str() );

               PropertyWritingContainer pc ( "TerrainType", tt->fileName + ".asctxt" );
               cout << "Writing file " << pc.getFilename() << "... ";
               tt->runTextIO ( pc );
               pc.run();

               cout << "done \n";

               delete tt;
            }
            if ( patimat ( "*.veh", filename.c_str() )) {
               Vehicletype* vt = loadvehicletype ( filename.c_str() );

               PropertyWritingContainer pc ( "VehicleType", vt->filename + ".asctxt" );
               cout << "Writing file " << pc.getFilename() << "... ";
               vt->runTextIO ( pc );
               pc.run();

               cout << "done \n";

               delete vt;
            }
            filename = ff.getnextname();
         }
      }

   } /* endtry */
   catch ( tfileerror err ) {
      printf("\nfatal error accessing file %s \n", err.getFileName().c_str() );
      return 1;
   } /* endcatch */
   catch ( ASCexception ) {
      printf("\na fatal exception occured\n" );
      return 2;
   } /* endcatch */
   
   return 0;
}


