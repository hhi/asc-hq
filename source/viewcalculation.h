/***************************************************************************
                          viewcalculation.h  -  description
                             -------------------
    begin                : Thu Oct 5 2000
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


#ifndef viewcalculation_h_included
 #define viewcalculation_h_included

 #include "typen.h"
 #include "mapalgorithms.h"
 #include "gamemap.h"

  class tcomputeview : public SearchFields {
                protected:
                      // pmap gamemap;
                      int actView;
                      int player;
                      int mode;
                      int height;

                      int sonar,satellitenview,minenview;
                      int        viewdist;
                      int        jamdist;
                      virtual void  initviewcalculation( int view, int jamming, int sx, int sy, int _mode, int _height   );   // mode: +1 = add view  ;  -1 = remove view
                      virtual void  testfield ( const MapCoordinate& mc );

                public:
                      tcomputeview ( pmap _actmap ) : SearchFields ( _actmap ) { actView = _actmap->playerView; };
                 };

  class tcomputevehicleview : public tcomputeview {
                           public:
                               tcomputevehicleview ( pmap actmap ) : tcomputeview ( actmap ) {};
                               void          init( const pvehicle eht, int _mode  );   // mode: +1 = add view  ;  -1 = remove view );
                           };

  class tcomputebuildingview : public tcomputeview  {
                              pbuilding         building;
                           public:
                              tcomputebuildingview ( pmap actmap ) : tcomputeview ( actmap ) {};
                              void              init( const pbuilding    bld, int _mode );
                           };

  extern int computeview( pmap actmap, int player_fieldcount_mask = 0 );
  extern int  evaluatevisibilityfield ( pmap actmap, pfield fld, int player, int add, int initial );

  extern int  evaluateviewcalculation ( pmap actmap, int player_fieldcount_mask = 0 );     // playermask determines, which players should be counted when the view has changed
                                                                // returns the number of fields which have changed visibilitystatus

  extern int  evaluateviewcalculation ( pmap actmap, int x, int y, int distance, int player_fieldcount_mask = 0 );     // playermask determines, which players should be counted when the view has changed
                                                                // returns the number of fields which have changed visibilitystatus


  extern void setvisibility ( word* visi, int valtoset, int actplayer );

#endif