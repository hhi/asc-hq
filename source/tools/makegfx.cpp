#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <map>

#include "../basegfx.h"
#include "../basestrm.h"
#include "../typen.h"
#include "../misc.h"
#include "../loadpcx.h"
#include "../palette.h"
#include "../sgstream.h"
#include "../basegfx.h"
#include "../strtmesg.h"

#ifdef WIN32
#undef main
#endif


typedef map<int,void*> IMGS;
IMGS images;

bool decodeFileName ( const ASCString& f, int& num )
{
   ASCString s = f;
   for ( int i = 0; i < s.length(); i++ ) {
      if ( s[i] == '.' ) {
         s.erase ( i );
         break;
      }
      if ( s[i] < '0' || s[i] > '9' )
         return false;
   }
   num = atoi ( s.c_str() );
   return true;
}


// including the command line parser, which is generated by genparse
#include "../clparser/makegfx.cpp"

int main(int argc, char *argv[] )
{
   Cmdline cl ( argc, argv );

   if ( cl.v() ) {
      cout << argv[0] << " " << getVersionString() << endl;
      exit(0);
   }

   if ( argc - cl.next_param() < 2 ) {
      cl.usage();
      exit(1);
   }

   try {

   initFileIO( cl.c().c_str() );
   addSearchPath ( "." );
   opencontainer ( "*.con");
   loadpalette();

   void* mask;
   {
      int i ;
      tnfilestream s ( "largehex.raw", tnstream::reading );
      s.readrlepict ( &mask, false, & i );
   }

   int highestImage = 0;

   char tempbuf[10000];

   for ( int i = cl.next_param()+1; i < argc; i++ ) {
      tfindfile ff ( argv[i] );
      ASCString filename = ff.getnextname();
      while ( !filename.empty() ) {
         int num;
         if ( decodeFileName ( filename, num ) ) {
            TrueColorImage* tci;
            extractPath ( tempbuf, argv[i] );
            ASCString file = tempbuf + filename;
            void* img;
            int depth = pcxGetColorDepth ( file.c_str() );
            if ( depth > 8 ) {
               {
                   tvirtualdisplay vdp ( 100, 100, TCalpha, 32 );
                   loadpcxxy ( file.c_str(), 0, 0, 0 );
                   tci = getimage ( 0, 0, fieldxsize-1, fieldysize-1 );
               } {
                   tvirtualdisplay vdp ( 100, 100, 255 );
                   img = convertimage ( tci, pal );
                   putimage ( 0, 0, img );
                   putmask ( 0, 0, mask, 0 );
                   getimage ( 0, 0, fieldsizex-1, fieldsizey-1, img );
               }
            } else {
               tvirtualdisplay vdp ( 100, 100, 255, 8 );
               loadpcxxy ( file.c_str(), 0, 0, 0 );
               putmask ( 0, 0, mask, 0 );
               img = new char[imagesize (0, 0, fieldsizex-1, fieldsizey-1)];
               getimage ( 0, 0, fieldsizex-1, fieldsizey-1, img );
            }
            images[num] = img;
            highestImage = max ( num, highestImage );

         }
         filename = ff.getnextname();
      }
   }

   tnfilestream s ( argv[cl.next_param()], tnstream::writing );
   int magic = -1;
   s.writedata2 ( magic );

   int id = 1;
   printf ("\n    ID :  \n ( 0 = original ASC graphics; 1 = BI3 graphics; >=2 : additional graphic sets)\n    ");
   scanf ( "%d", &id );
   // num_ed ( id , 0, 65534);

   s.writeInt ( id );

   highestImage++;

   int* picmode = new int[highestImage];
   for ( int i = 0; i < highestImage; i++ )
      picmode[i] = 0;

   int maxpicsize = 0;
   for ( IMGS::iterator ii = images.begin(); ii != images.end(); ii++ ) {
      picmode[ii->first] =  2;
      if ( maxpicsize < getpicsize2 ( ii->second ) )
          maxpicsize = getpicsize2 ( ii->second );
   }

   s.writeInt ( highestImage );
   s.writeInt ( maxpicsize );
   s.writedata ( picmode, highestImage * sizeof ( int ) );
   for ( IMGS::iterator ii = images.begin(); ii != images.end(); ii++ )
      s.writedata ( ii->second, getpicsize2 ( ii->second ) );

   }
   catch ( tfileerror err ) {
      fatalError( "file error accessing file %s ", err.getFileName().c_str() );
   } /* endcatch */
   catch ( ASCexception) {
      fatalError( "unspecified error" );
   } /* endcatch */
   return 0;
}
