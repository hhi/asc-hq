/***************************************************************************
                          events.h  -  description
                             -------------------
    begin                : Wed Oct 18 2000
    copyright            : (C) 2000 by Martin Bickel
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


/***************************************************************************
 *                                                                         *
 *   Event handling routines                                               *
 *                                                                         *
 ***************************************************************************/

#ifndef events_h_included
#define events_h_included


extern void initializeEventHandling ( int (*fn)(void *) , void *data, void* mousepointer );

extern SDL_mutex* eventHandlingMutex;

/***************************************************************************
 *                                                                         *
 *   Mouse handling routines                                               *
 *                                                                         *
 ***************************************************************************/


struct tmouserect {
       int x1, y1;
       int x2, y2;
       tmouserect operator+ ( const tmouserect& b ) const;
//       tmouserect ( int _x1, int _y1, int _x2, int _y2 ) : x1(_x1), y1(_y1), x2(_x2), y2(_y2 ) {};
//       tmouserect(){ x1=0;y1=0;x2=0;y2=0;};
};

extern void mousevisible( int an );
extern int getmousestatus ();

extern void setmouseposition ( int x, int y );

extern void setinvisiblemouserectangle ( int x1, int y1, int x2, int y2 );
// -1, -1, -1, -1  schaltet die Maus wieder ?berall an

extern void setinvisiblemouserectanglestk ( int x1, int y1, int x2, int y2 );
extern void setinvisiblemouserectanglestk ( tmouserect r1 );
// wie oben, jedoch werden die alten Params auf dem Stack gesichert

extern void getinvisiblemouserectanglestk ( void );
// holt die Params wieder vom Stack

extern void setnewmousepointer ( void* picture, int hotspotx, int hotspoty );

extern int mouseinrect ( int x1, int y1, int x2, int y2 );


extern int mouseinrect ( const tmouserect* rect );

class tmousesettings {
  public:
   int x;
   int y;
   int x1;
   int y1;
   int altx;
   int alty;
   void *background;
   void *pictpointer;
   int xsize;
   int ysize;
   char taste;
   char status;
   tmouserect off;
   int hotspotx;
   int hotspoty;
   int backgroundsize;
   tmousesettings ( ) { x=0;y=0;x1=0;y1=0;altx=0;alty=0;
                        background=NULL;pictpointer=NULL;
                        xsize=0;ysize=0;taste=0;status=0;
                        hotspotx=0;hotspoty=0;backgroundsize=0;
                        off.x1=0;off.y1=0;off.x2=0;off.y2=0;
                      };
};




class tsubmousehandler {
        protected:
           tmouserect offarea;
        public:
           tsubmousehandler ( void ) { offarea.x1 = 0; offarea.y1 = 0; offarea.x2 = 0; offarea.y2 = 0; };
           virtual void mouseaction ( void ) = 0;
           virtual void invisiblerect ( tmouserect newoffarea ) { offarea = newoffarea; };
     };


extern void mouseintproc2( void );
extern volatile tmousesettings mouseparams;


extern void addmouseproc ( tsubmousehandler* proc );
extern void removemouseproc ( tsubmousehandler* proc );

extern void pushallmouseprocs ( void );
extern void popallmouseprocs ( void );

#ifdef _DOS_
extern int initmousehandler ( void* pic );
extern void removemousehandler ();
#endif


/***************************************************************************
 *                                                                         *
 *   Keyboard handling routines                                            *
 *                                                                         *
 ***************************************************************************/


#ifdef _DOS_
 #include "dos/keysymbols.h"
#else
 #ifdef _SDL_
  #include "sdl/keysymbols.h"
 #endif
#endif

#ifdef _DOS_
extern void initkeyb();
extern void closekeyb();
#endif

 typedef int tkey;

 extern char  skeypress( tkey keynr);
 extern char *get_key(tkey keynr);
 extern tkey r_key(void);
 extern int  rp_key(void);
 extern int keypress(void);
 extern void wait(void);
 extern tkey char2key (int ch);
 extern void getkeysyms ( tkey* keysym, int* keyprnt );

 extern int exitprogram;


/***************************************************************************
 *                                                                         *
 *   Timer routines                                                        *
 *                                                                         *
 ***************************************************************************/


#ifdef _DOS_
    volatile extern long        ticker;
#else
    extern volatile int ticker;
#endif
    extern void ndelay(int time);

    extern void starttimer(void); //resets Timer
    extern char time_elapsed(int time); //check if time msecs are elapsed, since starttimer
    extern int  releasetimeslice( void );


#endif
