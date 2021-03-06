/***************************************************************************
                          dashboard.h  -  description
                             -------------------
    begin                : Sat Jan 27 2001
    copyright            : (C) 2001 by Martin Bickel
    email                : bickel@asc-hq.org
 ***************************************************************************/

/*! \file dashboard.h
    \brief The box displaying unit information
*/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#ifndef dashboardH
 #define dashboardH

#include "paradialog.h"
#include "windowing.h"

class ContainerBase;
class VehicleType;
class Vehicle;
class Building;
class SingleWeapon;
class MapDisplay;
class GameMap;
class MapField;

class DashboardPanel : public LayoutablePanel {
   private:
      void containerDeleted( ContainerBase* c );
   protected:
       Vehicle* veh;
       Building* bld;
       
      DashboardPanel ( PG_Widget *parent, const PG_Rect &r, const ASCString& panelName_, bool loadTheme );

      void painter ( const PG_Rect &src, const ASCString& name, const PG_Rect &dst);
      void registerSpecialDisplay( const ASCString& name );

      void reset(GameMap& map);

      bool containerRenamed( PG_LineEdit* lineEdit );

      bool viewExperienceOverview();

   public:
      void eval();
      void showUnitData( Vehicle* veh, Building* bld, MapField* fld, bool redraw = false );

};

class WindInfoPanel : public DashboardPanel {
        Surface windArrow;
        int dir;
     protected:
        void painter ( const PG_Rect &src, const ASCString& name, const PG_Rect &dst);
     public:
        WindInfoPanel (PG_Widget *parent, const PG_Rect &r ) ;
};

class UnitInfoPanel : public DashboardPanel {
     protected:
        bool onClick ( PG_MessageObject* obj, const SDL_MouseButtonEvent* event );
        void showUnitInfo( const VehicleType* vt );
        bool unitNaming();
     public:
        UnitInfoPanel (PG_Widget *parent, const PG_Rect &r ) ;
};


class MapDisplayPG;

class MapInfoPanel : public DashboardPanel {
        MapDisplayPG* mapDisplay;
        PG_Slider* zoomSlider;
        bool changeActive;
        
        void layerChanged( bool state, const ASCString& label );        
        bool scrollTrack( long pos );
        bool checkBox( bool state, const char* name );
        void zoomChanged( int zoom );

        bool showWeaponRange();
        bool showMovementRange();
   protected:
        void painter ( const PG_Rect &src, const ASCString& name, const PG_Rect &dst);
     public:
        MapInfoPanel (PG_Widget *parent, const PG_Rect &r, MapDisplayPG* mapDisplay ) ;
};


class ActionInfoPanel : public DashboardPanel {
   public:
      ActionInfoPanel (PG_Widget *parent, const PG_Rect &r ) ;
      
      void update( GameMap* map );
};

#endif
