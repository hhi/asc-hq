//     $Id: cdrom.h,v 1.3 2009-04-18 13:48:38 mbickel Exp $
//
//     $Log: not supported by cvs2svn $
//     Revision 1.2  2000/08/12 12:52:56  mbickel
//      Made DOS-Version compile and run again.
//
//     Revision 1.1  2000/05/30 18:39:28  mbickel
//      Added support for multiple directories
//      Moved DOS specific files to a separate directory
//
//     Revision 1.4  2000/05/23 20:40:37  mbickel
//      Removed boolean type
//
//     Revision 1.3  2000/04/27 16:25:16  mbickel
//      Attack functions cleanup
//      New vehicle categories
//      Rewrote resource production in ASC resource mode
//      Improved mine system: several mines on a single field allowed
//      Added unitctrl.* : Interface for vehicle functions
//        currently movement and height change included
//      Changed timer to SDL_GetTicks
//
//     Revision 1.2  1999/11/16 03:41:14  tmwilson
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

#ifdef _DOS_
#include <i86.h>
#include <conio.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../tpascal.inc"
#include "realint.h"

     typedef char tdevname[8]; 
     struct trequestheader { 
                       pascal_byte         len; 
                       pascal_byte         subunit; 
                       pascal_byte         commandcode; 
                       word         status; 
                       tdevname  devname; 
                    } ;


     struct tgetdevheader {
                     pascal_byte         controlblockcode; 
                     void*      adress; 
                  } ;

     struct tioctlo { 
                trequestheader requestheader; 
                pascal_byte         mediadescriptor; 
                int            buffer;
                word         buffersize; 
                int      startsector; 
                int      volumeptr; 
             };

     struct tlocatehead {
                pascal_byte         controlblockcode;   /* 1 */ 
                pascal_byte         adressingmode; 
                int      headlocation; 
             } ; 

     struct taudiochannel {   /* 4 */
                      pascal_byte         controlblockcode; 
                      pascal_byte         inputchannel0; 
                      pascal_byte         volumechannel0; 
                      pascal_byte         inputchannel1; 
                      pascal_byte         volumechannel1; 
                      pascal_byte         inputchannel2; 
                      pascal_byte         volumechannel2; 
                      pascal_byte         inputchannel3; 
                      pascal_byte         volumechannel3; 
                   } ;

      struct tdevicestatus {
                      pascal_byte         controlblockcode;   /* 6 */ 
                      int      status; 
                   };

      struct tcdsize {
                pascal_byte         controlblockcode;   /* 8 */ 
                int      size; 
             } ;

      struct tcdinfo {
                pascal_byte         controlblockcode;   /* 10 */ 
                pascal_byte         lowesttrack; 
                pascal_byte         highesttrack; 
                int      leadout; 
             };

      struct ttrackinfo {
                   pascal_byte         controlblockcode;   /* 11 */ 
                   pascal_byte         tracknumber; 
                   int      startpoint; 
                   pascal_byte         controlinfo;
                } ;

      struct tqinfo {
               pascal_byte         controlblockcode;   /* 12 */ 
               pascal_byte         caab; 
               pascal_byte         poi; 
               pascal_byte         tracknumber; 
               pascal_byte         min; 
               pascal_byte         sec; 
               pascal_byte         frame; 
               pascal_byte         zero; 
               pascal_byte         amin; 
               pascal_byte         asec; 
               pascal_byte         aframe; 
            };

      struct tlockdoor {
                  pascal_byte         controlblockcode;   /* 1 */ 
                  pascal_byte         lockfunction; 
               } ; 

      struct tseek {   /* 131 */
              trequestheader requestheader; 
              pascal_byte         adressingmode; 
              int      transferadress; 
              word         numberofsectors; 
              int      startsector; 
           } ; 

      struct tplayaudio {   /* 132 */
                   trequestheader requestheader; 
                   pascal_byte         adressingmode; 
                   int      startsector; 
                   int      numberofsectors; 
                } ; 

     struct tstopresume {   /* 133,134 */
                   trequestheader requestheader; 
                } ; 

     struct ttrackinf {
            char name[30];
            pascal_byte min,sec,frame;
            int start;
            pascal_byte smin,ssec,sframe;
            int size;
            pascal_byte type;
        };

     typedef ttrackinf *ptrackinfo;
     struct tcdinf {
            pascal_byte min,sec,frame;
            int size;
            pascal_byte smin,ssec,sframe;
            int start;
            pascal_byte firsttrack,lasttrack;
            char name[50];
            ptrackinfo track[99];
        } ;

     struct taudioinfo {
        pascal_byte volume[3];
        pascal_byte channel[3];
     };

#define tt2c 0
#define tt2cp 1
#define tt4c 2
#define tt4cp 3
#define ttd 4

const short int cdromintmemsize=60;

class tcdrom {
      tioctlo *ioctl;
      taudiochannel *ac;
      tdevicestatus *ds;
   public :
      char activecdrom;
      pascal_byte error;
      char numberofdrives;
      char driveletter[16];
      taudioinfo ta;
      tcdinf cdinfo;

      tcdrom(void);
      ~tcdrom(void);

      void* getdevheaderadress(void);
      char testcdromavailable(void);
      pascal_byte geterror( void );
      pascal_byte checkerror( void );
      char testcdromopen(void);
      void getcdrominfo(void);
      void getcdromdrives(void);
      void changecdromdrive(char nr);
      float get_mscdex_version(void);
      char openclosecdrom(void);
      char lockunlockcdrom(void);
      void getsectortime(int sector,pascal_byte *m,pascal_byte *s,pascal_byte *f);
      int getnormalsector(pascal_byte m,pascal_byte s,pascal_byte f);
      char checkbusy(void);
      int getheadlocation(void);
      void seeksector(int ss);
      void getactivetimes(pascal_byte *min,pascal_byte *sec,pascal_byte *frame,pascal_byte *amin,pascal_byte *asec,pascal_byte *aframe);
      int getcdsize(void);
      void getcdlength(pascal_byte *min,pascal_byte *sec,pascal_byte *frame);
      void getcdinfo(pascal_byte *l,pascal_byte *h,pascal_byte *min,pascal_byte *sec,pascal_byte *frame);
      void gettracklength(pascal_byte tracknr,pascal_byte *min,pascal_byte *sec,pascal_byte *frame);
      void gettrackinfo(pascal_byte tracknr,pascal_byte * min,pascal_byte * sec,pascal_byte * frame,pascal_byte * type);
      void getaudioinfo( void );
      void setaudiochannel( void );
      void playaudio(int ss,   int numbersectors);
      void stopaudio(void);
      void resumeaudio(void);

      void readcdinfo( void );
      void playtrack(pascal_byte nr);
      void playtrackuntilend(pascal_byte nr);
};

