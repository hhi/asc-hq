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

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef HAVE_STRING_H
#  include <string.h>
#elif #ifdef HAVE_STRINGS_H
#  include <strings.h>
#endif

#include <SDL_types.h>

#include "../basestrm.h"
#include "../strtmesg.h"
#include "../ascstring.h"
#include "../errors.h"

#ifdef logging
 void addToLog ( const ASCString& str )
 {
   FILE* f = fopen ( "demount.log", "at");
   fprintf(f, str.c_str() );
   fprintf(f, "\n" );
   fclose(f);
 }
#else
  #define addToLog(a)
#endif

// including the command line parser, which is generated by genparse
#include "../clparser/demount.cpp"

int main(int argc, char *argv[] )
{
   addToLog( "Demount started\n");
   
   Cmdline cl ( argc, argv );

   if ( cl.v() ) {
      cout << argv[0] << " " << getVersionString() << endl;
      exit(0);
   }

   if ( argc == cl.next_param() ) {
      cl.usage();
      exit(1);
   }


   Uint32 pos;
   Uint32 num = 0;
   for ( int a = cl.next_param(); a < argc; a++ ) {

      addToLog( ASCString("Processing ") + argv[a] );

      FILE* fp = fopen ( argv[a], filereadmode );
      if ( !fp )
         fatalError ( ASCString("Could not open file ") + argv[a] + " for reading!" );

      /* 64 bit problem ? */
      char magic[4];
      fread(&magic,1,sizeof(magic),fp);

      if (strncmp(magic,"NCBM",4) != 0)
         fatalError("Invalid containerfile: magic not matched");

      addToLog( "Reading pos" );

      /* mikem: somehow I don't like the order */
      fread ( &pos, sizeof(pos), 1, fp );

      addToLog( "Seeking to pos" );

      fseek ( fp, pos, SEEK_SET );

      addToLog( "Reading count" );

      fread ( &num, sizeof(num), 1, fp );

      addToLog( "Allocating index memory" );

      tcontainerindex* index = new tcontainerindex[num];

      addToLog( "Reading index" );

      Uint32 i;
      for (i = 0; i < num; i++ ) {
         fread ( &index[i], 1, sizeof ( index[i] ) , fp );
         if ( index[i].name ) {
            int p = -1;
            index[i].name = new char[100];
            do {
               fread ( &index[i].name[++p], 1, 1, fp );
            } while ( index[i].name[p] ); /* enddo */
         }
      }

      addToLog( "Closing index" );
      fclose ( fp );

      addToLog( "Opening container pos" );
      opencontainer ( argv[a] );

      Uint32 bufsize = 1000000;

      addToLog( "Reserving buffer" );

      void* buf;
      if ( ! (buf = malloc ( bufsize )) ) {
	printf( "couldn't allocate buffer\n" );
	exit(1);
      }

      addToLog ( ASCString("Number of files: ") + strrr(num));

      for ( i = 0; i < num; i++ ) {
         try {
            addToLog( ASCString("Finding file ") + strrr ( i+1 ));

            tfindfile ff ( index[i].name );
            bool incontainer;
            ASCString nme = ff.getnextname ( NULL, &incontainer );
            if ( incontainer || nme.empty() ) {
               addToLog( "Reserving buffer" );
               tnfilestream instream ( index[i].name, tnstream::reading );
               char namebuf[ maxFileStringSize ];
               int j = 0;
               do {
		 //j++;
                  namebuf[j] = tolower ( index[i].name[j] );
               } while ( namebuf[j++] );

               addToLog( "filename converted" );

               if ( !cl.q() )
                  printf("writing %s \n", namebuf );

               addToLog( "opening output stream" );
               tn_file_buf_stream outstream ( namebuf, tnstream::writing );
               Uint32 size ;
               do { 
		  size = instream.readdata ( buf, bufsize, 0 );
                  outstream.writedata ( buf, size );
               } while ( size == bufsize );
               addToLog( "data written" );
            } else {
               printf("file %s already exists; skipping\n", index[i].name );
               addToLog( ASCString("File ") + index[i].name + " skipped" );
            }
         }
         catch ( tfileerror err ) {
            fatalError( "error writing file " + err.getFileName() );
         } /* endcatch */
         catch ( ... ) {
            fatalError( "Exception caught !" );
         } /* endcatch */

      } /* endfor */

      addToLog( "freeing buffer" );
      free ( buf );
   }
   addToLog( "quitting" );
   return 0;
}
