/*! \file mapalgorithms.cpp
    \brief Routines for working with hexagonal grids
*/

/***************************************************************************
                          mapalgorithms.cpp  -  description
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

#include <math.h>
#include "mapalgorithms.h"
#include "typen.h"
#include "vehicletype.h"
#include "errors.h"
#include "gamemap.h"

tdrawgettempline :: tdrawgettempline ( int _freefields, GameMap* _gamemap )
{
   gamemap = _gamemap;
   tempsum = 0;
   freefields = _freefields;
   num = 0;
}

void tdrawgettempline :: putpix8 ( int x, int y )
{
   if ( !gamemap->getField ( x, y ) )
      return;
   
   if ( num >= freefields )
      tempsum += gamemap->getField ( x, y )->getjamming();
   num++;
   /*
                           getfield( x,y )->temp = 100;
                           displaymap();
                           cursor.gotoxy ( x , y );
                                                   */
}

int tdrawgettempline :: winkelcomp ( double w1, double w2 )
{
   double pi = 3.141592654;
   double delta = w2 - w1;
   if ( delta > -0.0001 && delta < 0.0001 )
      return 0;

   if ( delta > 0 ) {
      if ( delta <= pi )
         return 1;
      else
         return -1;
   }

   if ( delta < 0 ) {
      if ( delta < -pi )
         return 1;
      else
         return -1;
   }

   return 0;
}


int tdrawgettempline :: initialized = 0;
double tdrawgettempline :: dirs[ sidenum ];
double tdrawgettempline :: offset;

void tdrawgettempline :: init ( void )
{
   if ( !initialized ) {
      offset = 0;
      sx = 10;
      sy = 10;
      int i;

      for ( i = 0; i < sidenum; i++ ) {
         sx = 10;
         sy = 10;
	 sx += getnextdx( i, sy );
	 sy += getnextdy( i );
         dirs[i] = winkel ( 10, 10 );
      }
      offset = dirs[0];

      for ( i = 0; i < sidenum; i++ ) {
         sx = 10;
         sy = 10;
	 sx += getnextdx( i, sy );
	 sy += getnextdy( i );
         dirs[i] = winkel ( 10, 10 );
      }

      initialized = 1;
   }
}


double tdrawgettempline :: winkel ( int x, int y )
{
   int xp2 = sx * fielddistx + (sy & 1) * fielddisthalfx;
   int yp2 = sy * fielddisty;

   int xp1 = x * fielddistx + (y & 1) * fielddisthalfx;
   int yp1 = y * fielddisty;

   int dx = xp2-xp1;
   int dy = yp2-yp1;
   double at = atan2 ( double(dy), double(dx) );
   // printf("%d / %d / %f \n", dx, dy, at);
   at -= offset;
   while ( at < 0 )
      at += 2 * 3.14159265;

   // printf("%f \n", at);
   return at;
}

#define checkvisibility

void tdrawgettempline :: start ( int x1, int y1, int x2, int y2 )
{
   init();

   sx = x2;
   sy = y2;

   if ( y1 == y2  && x1 == x2 )
      return;

   int x = x1;
   int y = y1;

   double w = winkel ( x1, y1 );

   int dir = -1;
   double mindiff = 10000;
   for ( int i = 0; i < sidenum; i++ ) {
      double nd = fabs ( dirs[i] - w );
      if ( nd < mindiff ) {
         dir = i;
         mindiff = nd;
      }
   }

#ifdef checkvisibility
   int ldist = beeline ( x1, y1, x2, y2 );
#endif

   int lastdir = winkelcomp ( w, dirs[dir] );
   /*
      if ( x1 == 18 && y1 == 24 && x2 == 18 && y2 == 9 ) {
         printf("blurb");
      }
   */

   x += getnextdx( dir, y );
   y += getnextdy( dir );
   while ( x != x2 || y != y2 ) {
#ifdef checkvisibility
      int ldist2 = beeline ( x, y, x2, y2 );
      if ( ldist2 > ldist ) {
         fatalError ( "inconsistency in tdrawgettempline :: start ; parameters are %d/%d ; %d/%d ", 1, x1, y1, x2, y2 );
         return;
      }
#endif

      putpix8 ( x, y );
      double w2 = winkel ( x, y );
      // printf("%f \n", w2);
      if ( lastdir > 0 ) {
         if ( winkelcomp ( w2,  w ) == 1 ) {
            dir--;
            lastdir = -1;
         }
      } else {
         if ( winkelcomp ( w2 , w ) == -1 ) {
            dir++;
            lastdir = 1;
         }
      }
      if ( dir < 0 )
         dir += sidenum;

      if ( dir >= sidenum )
         dir = dir % sidenum;

      x += getnextdx( dir, y );
      y += getnextdy( dir );
   }
   putpix8 ( x, y );
}






SearchFields :: SearchFields ( GameMap* _gamemap )
{
   gamemap = _gamemap;
   cancelSearch = false;
}

void         SearchFields::initsearch( const MapCoordinate& startPosition, int _firstDistance, int _lastDistance )
{
   cancelSearch = false;
   startPos = startPosition;
   firstDistance = _firstDistance;
   lastDistance = _lastDistance;
}



void         SearchFields::startsearch(void)
{
   if ( cancelSearch )
      return;

   int   step;

   if (firstDistance > lastDistance)
      step = -1;
   else
      step = 1;

   dist = firstDistance;

   do {
      MapCoordinate mc ( startPos.x, startPos.y - 2 * dist );
      if ( dist == 0 ) {
         if ((mc.x >= 0) && (mc.y >= 0) && (mc.x < gamemap->xsize) && (mc.y < gamemap->ysize))
            testfield( mc );

         if ( cancelSearch )
            return;

      } else {
         for ( int e = 0; e < 6; e++ ) {
            int dir = (e + 2) % sidenum;
            for ( int c = 0; c < dist; c++) {
               if ((mc.x >= 0) && (mc.y >= 0) && (mc.x < gamemap->xsize) && (mc.y < gamemap->ysize)) {
                  testfield( mc );
                  if ( cancelSearch )
                     return;
               }
	       mc.x += getnextdx( dir, mc.y );
	       mc.y += getnextdy( dir );
            }

         }
      }

      dist += step;

   }  while (!((dist - step == lastDistance) || cancelSearch));
}


class SearchFieldsIterator : public SearchFields {
   public:
      typedef FieldIterationFunctor MyFunctor;
   private:   
      MyFunctor& myFunctor;
   protected:
      void testfield ( const MapCoordinate& pos ) {
         myFunctor(pos);
      };      
   public: 
      SearchFieldsIterator ( GameMap* _gamemap, MyFunctor& functor ) : SearchFields( _gamemap ), myFunctor( functor ) {};
           
};

void circularFieldIterator( GameMap* gamemap, const MapCoordinate& center, int startDist, int stopDist, FieldIterationFunctor functor )
{
   SearchFieldsIterator searchFields( gamemap, functor );
   searchFields.initsearch( center, startDist, stopDist );
   searchFields.startsearch();      
}


int         ccmpheighchangemovedir[6]  = {0, 1, 5, 2, 4, 3 };


MapCoordinate3D getNeighbouringFieldCoordinate( const MapCoordinate3D& pos, int direc)
{
  MapCoordinate3D mc = pos;
  mc.x += getnextdx( direc, mc.y );
  mc.y += getnextdy( direc );
  return mc;
}

MapCoordinate getNeighbouringFieldCoordinate( const MapCoordinate& pos, int direc)
{
  MapCoordinate mc = pos;
  mc.x += getnextdx( direc, mc.y );
  mc.y += getnextdy( direc );
  return mc;
}

int getdirection( const MapCoordinate& start, const MapCoordinate& dest )
{
   return getdirection(start.x, start.y, dest.x, dest.y );
}

int          getdirection(    int      x1,
                              int      y1,
                              int      x2,
                              int      y2)
{
   int dy = y2 - y1;
   int dx = x2 - x1;
   if (dx < 0)
      return (dy < 0) ? 5 : 4;
   if (dx > 0)
      return (dy < 0) ? 1 : 2;
   // dx == 0
   int dOdd = (y2 & 1) - (y1 & 1);
   if (dOdd == -1)
      return (dy < 0) ? 5 : 4;
   if (dOdd == 1)
      return (dy < 0) ? 1 : 2;
   // dOdd == 0
   return (dy < 0) ? 0 : (dy > 0) ? 3 : -1;
}


int beeline ( const Vehicle* a, const Vehicle* b )
{
   return beeline ( a->xpos, a->ypos, b->xpos, b->ypos );
}

int beeline ( const MapCoordinate& a, const MapCoordinate& b )
{
   return beeline ( a.x, a.y, b.x, b.y );
}


int beeline ( int x1, int y1, int x2, int y2 )
{
   int xdist = abs ( (x1 * 2 + (y1 & 1 )) - ( x2 * 2 + ( y2 & 1)) );
   int ydist = abs ( y2 - y1 );
   int num2;
   if ( ydist > xdist )
      num2 = (ydist - xdist) / 2 + xdist;
   else
      num2 = max ( xdist, ydist );
/*
   int num = 0;
   while ( x1 != x2  || y1 != y2 ) {
      num++;
      int direc = getdirection (x1, y1, x2, y2);
      x1 += getnextdx ( direc, y1 );
      y1 += getnextdy ( direc );
   }

   if ( num != num2 )
      printf("beeline inconsistent\n" );
*/
   return minmalq*num2;
}


inline int square ( int i )
{
   return i*i;
}

inline float square ( float i )
{
   #ifdef __GNUC__
   return __builtin_powif(i, 2);
   #else
   return pow(i, 2);
   #endif
}


WindMovement::WindMovement ( const Vehicle* vehicle )
{
   for ( int i = 0; i < sidenum; i++ )
      wm[i] = 0;

   int movement = 0;
   for ( int height = 4; height <= 6; height++ ){
      if ( vehicle->typ->movement[height] )
         if ( vehicle->typ->movement[height] > movement )
            movement = vehicle->typ->movement[height];
   }

   if ( movement ) {
      int wmn[7];
      
      int lastDir = 0;
      
      float abswindspeed = float( vehicle->getMap()->weather.windSpeed) * maxwindspeed / 255;
      
      for ( float direc = 0; direc < 360; direc++) {
         static const float pi = 3.14159265;
         float unitspeedx = movement * sin(direc/180*pi);
         float unitspeedy = movement * cos(direc/180*pi);

         float angle = atan2( unitspeedx, unitspeedy + abswindspeed );
         if ( angle < 0 )
            angle += 2 * pi;

         if ( angle >= 60 * float(lastDir) * (2*pi) / 360 ) {
            float absspeed = sqrt ( square ( unitspeedy + abswindspeed)+ square ( unitspeedx) );
            wmn[lastDir] = int( 10 - 10*movement/absspeed );
            ++lastDir;
         }
      }
            
      
      for ( int i = 0; i <= 3; i++ ) {
         wm[(i+vehicle->getMap()->weather.windDirection)%6] = wmn[i];
         if ( i > 0 )
            wm[(6-i+vehicle->getMap()->weather.windDirection)%6] = wmn[i];
      }
   }
}


/*
Wenn Du Wind aus Westen hast und willst nach norden fliegen, dann darf der Flieger nicht Kurs auf Norden nehmen, denn dann w�rde er abgetrieben. Er muss stattdessen etwas gegen den Wind fliegen. 

Diesen Winkel ermittel' ich durch plumpes ausprobieren, in WindMovement::WindMovement

Solange die Sollrichtung der Einheitenbewegung und die WindRichtungen �bereinstimmen, brauche ich nur den Fall f�r 1 Windrichtung berechnen. Ich ermittel' dann f�r die 6 Einheitenrichtungen den Malus. F�r andere Windrichtungen verschiebe ich einfach nur die Werte in die Tabelle (die letzten beiden Anweisungen in WindMovement), da das entscheidende ja nur der Winkelunterschied zwischen Wind- und Sollbewegungsrichtung ist.
*/

