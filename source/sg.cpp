/***************************************************************************
                           sg.cpp  -  description
                             -------------------
    begin                : a long time ago...
    copyright            : (C) 1994-2003 by Martin Bickel
    email                : bickel@asc-hq.org
 ***************************************************************************/

/*! \file sg.cpp
    \brief THE main program: ASC
*/


/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


/*!
   \mainpage 

   \section a short walk through the source
 
   THE central class of ASC is tmap in gamemap.h . 
   It is the anchor where nearly all elements of ASC are chained to. The global 
   variable #actmap is a pointer to the active map. There can be a maximum of
   8 players on a map, plus neutral units (which are handled like a 9th player). 
   Hence the array of 9 tmap::Player classes in tmap. 
   
   Each player has units and buildings, which are stored in the lists 
   tmap::Player::vehicleList and tmap::Player::buildingList . 
   The terms units and vehicles are used synonymously in ASC. Since unit was a 
   reserved word in Borland Pascal, we decided to use the term vehicle instead. 
   But now, with ASC written in C++,  'unit' is also used.
   
   Every building and unit is of a certain 'type': Vehicletype and BuildingType .
   These are stored in the data files which are loaded on startup and are globally 
   available. They are not modified during runtime in any way and are referenced 
   by the instances of Vehicle and Building. The Vehicletype has information that are shared
   by all vehicles of this 'type', like speed, weapon systems, accessable
   terrain etc, while the vehicle stores things like remaining movement for this
   turn, ammo, fuel and cargo.
   
   The primary contents of a map are its fields ( tfield). Each field has again a pointer 
   to a certain weather of a TerrainType. Each TerrainType has up to 5 
   different weathers ("dry (standard)","light rain", "heavy rain", "few snow",
   "lot of snow"). If there is a unit or a building standing on a field, the field
   has a pointer to it: tfield::vehicle and tfield::building .
   
   On the field can be several instances of Object. Objects are another central class of 
   ASC. Roads, pipleines, trenches and woods are examples of objects.
 
*/

#include "global.h"

#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <new>
#include <cstdlib>
#include <ctype.h>
#include <signal.h>
#include <algorithm>
#include <memory>
#include <SDL_image.h>
#include <SDL_mixer.h>

#include "paradialog.h"

#include "vehicletype.h"
#include "buildingtype.h"
#include "ai/ai.h"
#include "basegfx.h"
#include "misc.h"
#include "loadpcx.h"
#include "newfont.h"
#include "events.h"
#include "typen.h"
#include "spfst.h"
#include "loaders.h"
#include "dlg_box.h"
#include "stack.h"
#include "controls.h"
#include "dlg_box.h"
#include "dialog.h"
#include "gui.h"
#include "pd.h"
#include "strtmesg.h"
#include "gamedlg.h"
#include "network.h"
#include "building.h"
#include "sg.h"
#include "soundList.h"
#include "gameoptions.h"
#include "loadimage.h"
#include "astar2.h"
#include "errors.h"
#include "password.h"
#include "password_dialog.h"
#include "viewcalculation.h"
#include "replay.h"
#include "graphicset.h"
#include "loadbi3.h"
#include "itemrepository.h"
#include "music.h"
#include "messagedlg.h"
#include "statisticdialog.h"
#include "clipboard.h"
#include "mapdisplay2.h"
#include "guiiconhandler.h"
#include "guifunctions.h"
#include "iconrepository.h"
#include "dashboard.h"

// #define MEMCHK

#include "memorycheck.cpp"



class tsgonlinemousehelp : public tonlinemousehelp
{
   public:
      tsgonlinemousehelp ( void );
};

tsgonlinemousehelp :: tsgonlinemousehelp ( void )
{
   helplist.num = 12;

   static tonlinehelpitem sghelpitems[12]  = {{{ 498, 26, 576, 36}, 20001 },
         {{ 498, 41, 576, 51}, 20002 },
         {{ 586, 26, 612, 51}, 20003 },
         {{ 499, 57, 575, 69}, 20004 },
         {{ 499, 70, 575, 81}, 20005 },
         {{ 577, 58, 610, 68}, 20006 },
         {{ 577, 70, 610, 80}, 20007 },
         {{ 502, 92, 531,193}, 20008 },
         {{ 465, 92, 485,194}, 20009 },
         {{ 551, 92, 572,193}, 20010 },
         {{ 586, 90, 612,195}, 20011 },
         {{ 473,agmp->resolutiony - ( 480 - 449 ), 601,agmp->resolutiony - ( 480 - 460 )}, 20016 }};

   for ( int i = 0; i< helplist.num; i++ ) {
      sghelpitems[i].rect.x1 = agmp->resolutionx - ( 640 - sghelpitems[i].rect.x1 );
      sghelpitems[i].rect.x2 = agmp->resolutionx - ( 640 - sghelpitems[i].rect.x2 );
   }

   helplist.item = sghelpitems;
}

tsgonlinemousehelp* onlinehelp = NULL;




class tsgonlinemousehelpwind : public tonlinemousehelp
{
   public:
      tsgonlinemousehelpwind ( void );
} ;

tsgonlinemousehelpwind :: tsgonlinemousehelpwind ( void )
{
   helplist.num = 3;

   static tonlinehelpitem sghelpitemswind[3]  = { {{ 501,224, 569,290}, 20013 },
         {{ 589,228, 609,289}, 20014 },
         {{ 489,284, 509,294}, 20015 }};

   for ( int i = 0; i< helplist.num; i++ ) {
      sghelpitemswind[i].rect.x1 = agmp->resolutionx - ( 640 - sghelpitemswind[i].rect.x1 );
      sghelpitemswind[i].rect.x2 = agmp->resolutionx - ( 640 - sghelpitemswind[i].rect.x2 );
   }

   helplist.item = sghelpitemswind;
}

tsgonlinemousehelpwind* onlinehelpwind = NULL;



pprogressbar actprogressbar = NULL;

#define messagedisplaytime 300





#define mmaintainence

bool maintainencecheck( void )
{
   int res = 0;
   if ( res )
      return true;

#ifdef maintainence
   int num = 0;

   for ( int i = 0; i < 8; i++ )
      if ( actmap->player[i].stat == Player::human  && actmap->player[i].exist())
         num++;

   if ( actmap->campaign )
      num++;

   if ( actmap->network )
      num++;

   if ( num <= 1 )
      return 1;
   else
      return 0;

#else
   return false;
#endif
}


void* loadpcx2raw( const ASCString& file )
{
   int pcxwidth,imgwidth;
   int pcxheight,imgheight;
   int depth = pcxGetColorDepth ( file, &pcxwidth, &pcxheight );
   if ( depth > 8 )
      fatalError(file + " could not be loaded: only 8 bit images supported");

   tvirtualdisplay vdp ( pcxwidth, pcxheight, 255, 8 );
   loadpcxxy ( file, 0, 0, 0, &imgwidth, &imgheight );
   void* img = new char[imagesize (0, 0, imgheight-1, imgwidth-1)];
   getimage ( 0, 0, imgwidth-1, imgheight-1, img );
   return img;
}




class Menu : public PG_MenuBar {

    PG_PopupMenu* currentMenu;
    typedef list<PG_PopupMenu*> Categories;
    Categories categories;

   public:
      Menu ( PG_Widget *parent, const PG_Rect &rect=PG_Rect::null);
      ~Menu();
      
   protected:
      void setup();   

   private:
      void addbutton(const char* name, int id );
      void addfield ( const char* name );
     
};



class MainScreenWidget : public PG_Widget {
    PG_Application& app;
public:
    MainScreenWidget( PG_Application& application );
    enum Panels { ButtonPanel, WindInfo, UnitInfo };
    void spawnPanel ( Panels panel );

protected:
    MapDisplayPG* mapDisplay;
    NewGuiHost* guiHost;
    Menu* menu;
//    void eventDraw (SDL_Surface* surface, const PG_Rect& rect);
//    void Blit ( bool recursive = true, bool restore = true );
    ~MainScreenWidget() { };
};

MainScreenWidget* mainScreenWidget = NULL;







void         loadMoreData(void)
{
   int          w;
   {
      tnfilestream stream ( "height2.raw", tnstream::reading );
      for (int i=0;i<3 ;i++ )
         for ( int j=0; j<8; j++)
            stream.readrlepict( &icons.height2[i][j], false, &w );
   }

   {
      tnfilestream stream ("farbe.raw",tnstream::reading);
      for (int i=0;i<8 ;i++ )
         stream.readrlepict( &icons.player[i], false, &w );
   }

   {
      tnfilestream stream ("allianc.raw",tnstream::reading);
      for ( int i=0;i<8 ;i++ ) {
         stream.readrlepict(   &icons.allianz[i][0], false, &w );
         stream.readrlepict(   &icons.allianz[i][1], false, &w );
         stream.readrlepict(   &icons.allianz[i][2], false, &w );
      } /* endfor */
   }

   {
      tnfilestream stream ("weapicon.raw",tnstream::reading);
      for ( int i=0; i<14 ;i++ )
         stream.readrlepict(   &icons.unitinfoguiweapons[i], false, &w );
   }

   {
      tnfilestream stream ("expicons.raw",tnstream::reading);
      for ( int i=0; i<=maxunitexperience ;i++ )
         stream.readrlepict(   &icons.experience[i], false, &w );
   }

   /*
   {
      tnfilestream stream ("hexinvi2.raw",tnstream::reading);
      stream.readrlepict(   &icons.view.va8, false, &w);
   }

   {
      tnfilestream stream ("hexinvis.raw",tnstream::reading);
      stream.readrlepict(   &icons.view.nv8, false, &w);
      void* u = uncompress_rlepict ( icons.view.nv8 );
      if ( u ) {
         asc_free( icons.view.nv8 );
         icons.view.nv8 = u;
      }
   }

   {
      tnfilestream stream ("fg8.raw",tnstream::reading);
      stream.readrlepict(   &icons.view.fog8, false, &w);
   }
     */
   {
      tnfilestream stream ("windrose.raw",tnstream::reading);
      stream.readrlepict(   &icons.windbackground, false, &w);
   }

   if ( actprogressbar )
      actprogressbar->point();
   {
      tnfilestream stream ("hexfld_a.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.mark.active, false, &w);
   }

   if ( actprogressbar )
      actprogressbar->point(); {
      tnfilestream stream ("hexfld.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.mark.inactive, false, &w);
   }

   {
      tnfilestream stream ("in_ach.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.mark.movein_active, false, &w);
   }

   {
      tnfilestream stream ("in_h.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.mark.movein_inactive, false, &w);
   }

   {
      tnfilestream stream ("build_ah.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.mark.repairactive, false, &w);
   }

   {
      tnfilestream stream ("build_h.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.mark.repairinactive, false, &w);
   }

   {
      tnfilestream stream ("hexbuild.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.container_window, false, &w);
   }

   if ( actprogressbar )
      actprogressbar->point();


   loadpalette();
   for (w=0;w<256 ;w++ ) {
      palette16[w][0] = pal[w][0];
      palette16[w][1] = pal[w][1];
      palette16[w][2] = pal[w][2];
      xlattables.nochange[w] = w;
   } /* endfor */

   if ( actprogressbar )
      actprogressbar->point();

   loadicons();

   if ( actprogressbar )
      actprogressbar->point();

   loadmessages();

   if ( actprogressbar )
      actprogressbar->point(); {
      tnfilestream stream ("waffen.raw",tnstream::reading);
      int num = stream.readInt();

      static int xlatselectweaponguiicons[12] = { 2, 7, 6, 3, 4, 9, 0, 5, 10, 11, 11, 11 };

      for ( int i = 0; i < num; i++ )
         stream.readrlepict(   &icons.selectweapongui[xlatselectweaponguiicons[i]], false, &w );
      stream.readrlepict(   &icons.selectweaponguicancel, false, &w );
      stream.readrlepict(   &icons.selectweapongui[12], false, &w );
   }

   {
      tnfilestream stream ("knorein.raw",tnstream::reading);
      stream.readrlepict(   &icons.guiknopf, false, &w );
   }

   {
      tnfilestream stream ("compi2.raw",tnstream::reading);
      stream.readrlepict(   &icons.computer, false, &w );
   }

   {
      tnfilestream stream ("pfeil-a0.raw",tnstream::reading);
      for (int i=0; i<8 ;i++ ) {
         stream.readrlepict(   &icons.pfeil2[i], false, &w );
      } /* endfor */
   }

   {
      tnfilestream stream ("gebasym2.raw",tnstream::reading);
      for ( int i = 0; i < 12; i++ )
         for ( int j = 0; j < 2; j++ )
            stream.readrlepict(   &icons.container.lasche.sym[i][j], false, &w );
   }

   {
      tnfilestream stream ("netcontr.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.subwin.netcontrol.start, false, &w );
      stream.readrlepict(   &icons.container.subwin.netcontrol.inactive, false, &w );
      stream.readrlepict(   &icons.container.subwin.netcontrol.active, false, &w );
   }

   {
      tnfilestream stream ("ammoprod.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.subwin.ammoproduction.start, false, &w );
      stream.readrlepict(   &icons.container.subwin.ammoproduction.button, false, &w );
      stream.readrlepict(   &icons.container.subwin.ammoproduction.buttonpressed, false, &w );
      for ( int i = 0; i < 4; i++ )
         stream.readrlepict(   &icons.container.subwin.ammoproduction.schieber[i], false, &w );
      stream.readrlepict(   &icons.container.subwin.ammoproduction.schiene, false, &w );
   }

   if ( actprogressbar )
      actprogressbar->point(); {
      tnfilestream stream ("resorinf.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.subwin.resourceinfo.start, false, &w );
   }

   {
      tnfilestream stream ("windpowr.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.subwin.windpower.start, false, &w );
   }

   {
      tnfilestream stream ("solarpwr.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.subwin.solarpower.start, false, &w );

   }

   {
      tnfilestream stream ("ammotran.raw", tnstream::reading);
      stream.readrlepict(   &icons.container.subwin.ammotransfer.start, false, &w );
      stream.readrlepict(   &icons.container.subwin.ammotransfer.button, false, &w );
      stream.readrlepict(   &icons.container.subwin.ammotransfer.buttonpressed, false, &w );
      for ( int i = 0; i < 4; i++ )
         stream.readrlepict(   &icons.container.subwin.ammotransfer.schieber[i], false, &w );
      stream.readrlepict(   &icons.container.subwin.ammotransfer.schieneinactive, false, &w );
      stream.readrlepict(   &icons.container.subwin.ammotransfer.schiene, false, &w );
      if ( dataVersion >= 2 ) {
         stream.readrlepict(   &icons.container.subwin.ammotransfer.singlepage[0], false, &w );
         stream.readrlepict(   &icons.container.subwin.ammotransfer.singlepage[1], false, &w );
         stream.readrlepict(   &icons.container.subwin.ammotransfer.plus[0], false, &w );
         stream.readrlepict(   &icons.container.subwin.ammotransfer.plus[1], false, &w );
         stream.readrlepict(   &icons.container.subwin.ammotransfer.minus[0], false, &w );
         stream.readrlepict(   &icons.container.subwin.ammotransfer.minus[1], false, &w );
      }

   }

   {
      tnfilestream stream ("research.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.subwin.research.start, false, &w );
      stream.readrlepict(   &icons.container.subwin.research.button[0], false, &w );
      stream.readrlepict(   &icons.container.subwin.research.button[1], false, &w );
      stream.readrlepict(   &icons.container.subwin.research.schieber, false, &w );
   }

   {
      tnfilestream stream ("pwrplnt2.raw",tnstream::reading);
      stream.readrlepict(   &icons.container.subwin.conventionelpowerplant.start, false, &w );
      stream.readrlepict(   &icons.container.subwin.conventionelpowerplant.schieber, false, &w );
      //stream.readrlepict(   &icons.container.subwin.conventionelpowerplant.button[1], false, &w );
   }





   int m; {
      tnfilestream stream ( "bldinfo.raw", tnstream::reading );
      stream.readrlepict( &icons.container.subwin.buildinginfo.start, false, &m );
      for ( int i = 0; i < 8; i++ )
         stream.readrlepict( &icons.container.subwin.buildinginfo.height1[i], false, &m );
      for ( int i = 0; i < 8; i++ )
         stream.readrlepict( &icons.container.subwin.buildinginfo.height2[i], false, &m );
      stream.readrlepict( &icons.container.subwin.buildinginfo.repair, false, &m );
      stream.readrlepict( &icons.container.subwin.buildinginfo.repairpressed, false, &m );
      stream.readrlepict( &icons.container.subwin.buildinginfo.block, false, &m );
   }


   {
      tnfilestream stream ("mining2.raw", tnstream::reading);
      stream.readrlepict(   &icons.container.subwin.miningstation.start, false, &w );
      stream.readrlepict(   &icons.container.subwin.miningstation.zeiger, false, &w );
      /*
      for ( i = 0; i < 2; i++ )
         stream.readrlepict(   &icons.container.subwin.miningstation.button[i], false, &w );
      for ( i = 0; i < 2; i++ )
         stream.readrlepict(   &icons.container.subwin.miningstation.resource[i], false, &w );
      for ( i = 0; i < 3; i++ )
         stream.readrlepict(   &icons.container.subwin.miningstation.axis[i], false, &w );
      for ( i = 0; i < 2; i++ )
         stream.readrlepict(   &icons.container.subwin.miningstation.pageturn[i], false, &w );
      stream.readrlepict(   &icons.container.subwin.miningstation.graph, false, &w );
      */
   }

   {
      tnfilestream stream ("mineral.raw", tnstream::reading);
      stream.readrlepict(   &icons.container.subwin.mineralresources.start, false, &w );
      stream.readrlepict(   &icons.container.subwin.mineralresources.zeiger, false, &w );
   }

   {
      tnfilestream stream ("tabmark.raw", tnstream::reading);
      stream.readrlepict (   &icons.container.tabmark[0], false, &w );
      stream.readrlepict (   &icons.container.tabmark[1], false, &w );
   }


   {
      tnfilestream stream ("traninfo.raw", tnstream::reading);
      stream.readrlepict(   &icons.container.subwin.transportinfo.start, false, &w );
      for ( int i = 0; i < 8; i++ )
         stream.readrlepict(   &icons.container.subwin.transportinfo.height1[i], false, &w );
      for ( int i = 0; i < 8; i++ )
         stream.readrlepict(   &icons.container.subwin.transportinfo.height2[i], false, &w );
      stream.readrlepict(   &icons.container.subwin.transportinfo.sum, false, &w );
   }

   if ( actprogressbar )
      actprogressbar->point(); {
      tnfilestream stream ("attack.raw", tnstream::reading);
      stream.readrlepict (   &icons.attack.bkgr, false, &w );
      icons.attack.orgbkgr = NULL;
   }

   {
      tnfilestream stream ("hexfeld.raw", tnstream::reading);
      stream.readrlepict ( &icons.fieldshape, false, &w );
   }

   {
      tnfilestream stream ("mapbkgr.raw", tnstream::reading);
      stream.readrlepict ( &icons.mapbackground, false, &w );
   }

   {
      tnfilestream stream ("hex2oct.raw", tnstream::reading);
      stream.readrlepict (   &icons.hex2octmask, false, &w );
   }

   {
      tnfilestream stream ("weapinfo.raw", tnstream::reading);
      for ( int i = 0; i < 5; i++ )
         stream.readrlepict (   &icons.weaponinfo[i], false, &w );
   }

   backgroundpict.load();

}

enum tuseractions { ua_repainthard,     ua_repaint, ua_help, ua_dispvehicleimprovement, ua_mainmenu, ua_mntnc_morefog,
                    ua_mntnc_lessfog,   ua_mntnc_morewind,   ua_mntnc_lesswind, ua_mntnc_rotatewind, ua_changeresourceview,
                    ua_benchgamewv,     ua_benchgamewov,     ua_viewterraininfo, ua_unitweightinfo,  ua_writemaptopcx,  ua_writescreentopcx,
                    ua_startnewsinglelevel, ua_changepassword, ua_gamepreferences, ua_bi3preferences,
                    ua_exitgame,        ua_newcampaign,      ua_loadgame,  ua_savegame, ua_setupalliances, ua_settribute, ua_giveunitaway,
                    ua_vehicleinfo,     ua_researchinfo,     ua_unitstatistics, ua_buildingstatistics, ua_newmessage, ua_viewqueuedmessages,
                    ua_viewsentmessages, ua_viewreceivedmessages, ua_viewjournal, ua_editjournal, ua_viewaboutmessage, ua_continuenetworkgame,
                    ua_toggleunitshading, ua_computerturn, ua_setupnetwork, ua_howtostartpbem, ua_howtocontinuepbem, ua_mousepreferences,
                    ua_selectgraphicset, ua_UnitSetInfo, ua_GameParameterInfo, ua_GameStatus, ua_viewunitweaponrange, ua_viewunitmovementrange,
                    ua_aibench, ua_networksupervisor, ua_selectPlayList, ua_soundDialog, ua_reloadDlgTheme, ua_showPlayerSpeed, ua_renameunit,
                    ua_statisticdialog, ua_viewPipeNet, ua_cancelResearch, ua_showResearchStatus, ua_exportUnitToFile, ua_viewButtonPanel, ua_viewWindPanel,
                    ua_clearImageCache, ua_viewUnitInfoPanel };



class MainMenuPullDown : public tpulldown
{
   public:
      void init ( void );
} ;

void         MainMenuPullDown :: init ( void )
{
   alwaysOpen = true;

   addfield ( "Glo~b~al" );
   addbutton ( "~O~ptions", ua_gamepreferences );
   addbutton ( "~M~ouse options", ua_mousepreferences );
   // addbutton ( "Select Music Play ~L~ist ", ua_selectPlayList );
   addbutton ( "~S~ound options", ua_soundDialog );
   addbutton ( "seperator", -1);
   addbutton ( "E~x~it�ctrl-x", ua_exitgame );


   addfield ("~G~ame");
   addbutton ( "New ~C~ampaign", ua_newcampaign);
   addbutton ( "~N~ew single Level�ctrl-n", ua_startnewsinglelevel );

   addbutton ( "~L~oad game�ctrl-l", ua_loadgame );
   addbutton ( "Continue network game�F3", ua_continuenetworkgame);
   addbutton ( "supervise network game", ua_networksupervisor );

   addfield ( "~H~elp" );
   addbutton ( "HowTo ~S~tart email games", ua_howtostartpbem );
   addbutton ( "HowTo ~C~ontinue email games", ua_howtocontinuepbem );
   addbutton ( "seperator", -1);
   addbutton ( "~K~eys", ua_help );

   addbutton ( "~A~bout", ua_viewaboutmessage );

   tpulldown :: init();
   setshortkeys();
}


void loadMap()
{
   ASCString s1;

   mousevisible(false);
   fileselectsvga(mapextension, s1, true );

   if ( !s1.empty() ) {
      mousevisible(false);
      cursor.hide();
      displaymessage("loading map %s",0, s1.c_str() );
      loadmap( s1.c_str() );
      actmap->startGame();

      next_turn();

      removemessage();
      if (actmap->campaign != NULL) {
         delete  ( actmap->campaign );
         actmap->campaign = NULL;
      }

      displaymap();
      updateFieldInfo();
      moveparams.movestatus = 0;
   }
   mousevisible(true);
}


void loadGame()
{
   mousevisible(false);

   ASCString s1;
   fileselectsvga(savegameextension, s1, true );

   if ( !s1.empty() ) {
      mousevisible(false);
      cursor.hide();
      displaymessage("loading %s ",0, s1.c_str());
      loadgame(s1.c_str() );
      removemessage();
      if ( !actmap || actmap->xsize == 0 || actmap->ysize == 0 )
         throw  NoMapLoaded();

      if ( actmap->network )
         setallnetworkpointers ( actmap->network );

      computeview( actmap );
      displaymap();
      updateFieldInfo();
      moveparams.movestatus = 0;
   }
   mousevisible(true);
}


void saveGame( bool as )
{
   ASCString s1;

   int nameavail = 0;
   if ( !actmap->preferredFileNames.savegame[actmap->actplayer].empty() )
      nameavail = 1;

   if ( as || !nameavail ) {
      mousevisible(false);
      fileselectsvga(savegameextension, s1, false );
   } else
      s1 = actmap->preferredFileNames.savegame[actmap->actplayer];

   if ( !s1.empty() ) {
      actmap->preferredFileNames.savegame[actmap->actplayer] = s1;

      mousevisible(false);
      cursor.hide();
      displaymessage("saving %s", 0, s1.c_str());
      savegame(s1.c_str());

      removemessage();
      displaymap();
      cursor.show();
   }
   mousevisible(true);
}



void         newcampaign(void)
{
   tchoosenewcampaign tnc;
   tnc.init();
   tnc.run();
   tnc.done();
}



void         newsinglelevel(void)
{
   tchoosenewsinglelevel tnc;

   tnc.init();
   tnc.run();
   tnc.done();
   // actmap->player[0].exist();
}

void         startnewsinglelevelfromgame(void)
{
   cursor.hide();
   newsinglelevel();
   if ( !actmap )
      throw NoMapLoaded();
   computeview( actmap );
   displaymap();
   cursor.show();
}



void loadStartupMap ( const char *gameToLoad=NULL )
{
   if ( gameToLoad && gameToLoad[0] ) {
      try {
         if ( patimat ( tournamentextension, gameToLoad )) {

            if( validateemlfile( gameToLoad ) == 0 )
               fatalError( "Email gamefile %s is invalid. Aborting.", gameToLoad );

            try {
               tnfilestream gamefile ( gameToLoad, tnstream::reading );
               tnetworkloaders nwl;
               nwl.loadnwgame( &gamefile );
               if ( actmap->network )
                  setallnetworkpointers ( actmap->network );
            } catch ( tfileerror ) {
               fatalError ( "%s is not a legal email game.", gameToLoad );
            }
         } else if( patimat ( savegameextension, gameToLoad )) {
            if( validatesavfile( gameToLoad ) == 0 )
               fatalError ( "The savegame %s is invalid. Aborting.", gameToLoad );

            try {
               loadgame( gameToLoad );
               computeview( actmap );
            } catch ( tfileerror ) {
               fatalError ( "%s is not a legal savegame. ", gameToLoad );
            }

         } else if( patimat ( mapextension, gameToLoad )) {
            if( validatemapfile( gameToLoad ) == 0 )
               fatalError ( "Mapfile %s is invalid. Aborting.", gameToLoad );

            try {
               loadmap( gameToLoad );
               if ( actmap->network )
                  setallnetworkpointers ( actmap->network );
            } catch ( tfileerror ) {
               fatalError ( "%s is not a legal map. ", gameToLoad );
            }
         } else
            fatalError ( "Don't know how to handle the file %s ", gameToLoad );

      }
      catch ( InvalidID err ) {
         displaymessage( err.getMessage().c_str(), 2 );
      } /* endcatch */
      catch ( tinvalidversion err ) {
         if ( err.expected < err.found )
            displaymessage( "File/module %s has invalid version.\nExpected version %d\nFound version %d\nPlease install the latest version from www.asc-hq.org", 2, err.getFileName().c_str(), err.expected, err.found );
         else
            displaymessage( "File/module %s has invalid version.\nExpected version %d\nFound version %d\nThis is a bug, please report it!", 2, err.getFileName().c_str(), err.expected, err.found );
      }
   } else {  // resort to loading defaults

      ASCString s = CGameOptions::Instance()->startupMap; 

      if ( s.empty() )
         s = "asc001.map";


      int maploadable;
      {
         tfindfile ff ( s );
         string filename = ff.getnextname();
         maploadable = validatemapfile ( filename.c_str() );
      }

      if ( !maploadable ) {

         tfindfile ff ( mapextension );
         string filename = ff.getnextname();

         if ( filename.empty() )
            displaymessage( "unable to load startup-map",2);

         while ( !validatemapfile ( filename.c_str() ) ) {
            filename = ff.getnextname();
            if ( filename.empty() )
               displaymessage( "unable to load startup-map",2);

         }
         s = filename;
      }

      loadmap(s.c_str());

      displayLogMessage ( 6, "initializing map..." );
      actmap->startGame();
      displayLogMessage ( 6, "done\n Setting up Resources..." );
      actmap->setupResources();
      displayLogMessage ( 6, "done\n" );
   }
}


void         startnextcampaignmap( int id)
{
   tcontinuecampaign ncm;
   ncm.init();
   ncm.setid(id);
   ncm.run();
   ncm.done();
}


void benchgame ( int mode )
{
   int t2;
   int t = ticker;
   int n = 0;
   do {
      if ( mode <= 1 ) {
         if ( mode == 1 )
            computeview( actmap );
         displaymap();
      } else
         copy2screen();

      n++;
      t2 = ticker;
   } while ( t + 1000 > t2 ); /* enddo */
   double d = 100 * n;
   d /= (t2-t);
   char buf[100];
   sprintf ( buf, "%3.1f", d );
   displaymessage2 ( " %s fps ", buf );
}

class WeaponRange : public SearchFields
{
   public:
      int run ( const Vehicle* veh );
      void testfield ( const MapCoordinate& mc )
      {
         gamemap->getField( mc )->tempw = 1;
      };
      WeaponRange ( pmap _gamemap ) : SearchFields ( _gamemap )
      {}
      ;
};

int  WeaponRange :: run ( const Vehicle* veh )
{
   int found = 0;
   if ( fieldvisiblenow ( getfield ( veh->xpos, veh->ypos )))
      for ( int i = 0; i < veh->typ->weapons.count; i++ ) {
         if ( veh->typ->weapons.weapon[i].shootable() ) {
            initsearch ( veh->getPosition(), veh->typ->weapons.weapon[i].maxdistance/minmalq, (veh->typ->weapons.weapon[i].mindistance+maxmalq-1)/maxmalq );
            startsearch();
            found++;
         }
      }
   return found;
}


void viewunitweaponrange ( const Vehicle* veh, tkey taste )
{
   if ( veh && !moveparams.movestatus  ) {
      actmap->cleartemps ( 7 );
      WeaponRange wr ( actmap );
      int res = wr.run ( veh );
      if ( res ) {
         displaymap();

         if ( taste != ct_invvalue ) {
            while ( skeypress ( taste )) {

               while ( keypress() )
                  r_key();

               releasetimeslice();
            }
         } else {
            int mb = mouseparams.taste;
            while ( mouseparams.taste == mb && !keypress() )
               releasetimeslice();
            while ( keypress() )
               r_key();
         }

         actmap->cleartemps ( 7 );
         displaymap();
      }
   }
}


void viewPipeNet( tkey taste )
{

   if ( !moveparams.movestatus ) {
      actmap->cleartemps ( 7 );
      TerrainBits tb = getTerrainBitType(cbpipeline);
      for ( int x = 0; x < actmap->xsize; ++x )
         for ( int y = 0; y < actmap->ysize; ++y ) {
             pfield fld = actmap->getField ( x, y );
             if ( fieldvisiblenow( fld ))
                if ( (fld->bdt & tb).any() || fld->building )
                   fld->a.temp = 1;
         }

      displaymap();

      if ( taste != ct_invvalue ) {
         while ( skeypress ( taste )) {
            while ( keypress() )
               r_key();

            releasetimeslice();
         }
      } else {
         int mb = mouseparams.taste;
         while ( mouseparams.taste == mb && !keypress() )
            releasetimeslice();

         while ( keypress() )
            r_key();
      }
      actmap->cleartemps ( 7 );
      displaymap();
   }
}


void viewunitmovementrange ( Vehicle* veh, tkey taste )
{
   if ( veh && !moveparams.movestatus && fieldvisiblenow ( getfield ( veh->xpos, veh->ypos ))) {
      actmap->cleartemps ( 7 );
      TemporaryContainerStorage tcs ( veh, true );
      veh->reactionfire.disable();
      veh->setMovement ( veh->typ->movement[log2(veh->height)]);
      int oldcolor = veh->color;
      veh->color = actmap->actplayer*8;
      VehicleMovement vm ( NULL, NULL );
      if ( vm.available ( veh )) {
         vm.execute ( veh, -1, -1, 0, -1, -1 );
         veh->color = oldcolor;
         if ( vm.reachableFields.getFieldNum()) {
            for  ( int i = 0; i < vm.reachableFields.getFieldNum(); i++ )
               if ( fieldvisiblenow ( vm.reachableFields.getField ( i ) ))
                  vm.reachableFields.getField ( i )->a.temp = 1;
            for  ( int j = 0; j < vm.reachableFieldsIndirect.getFieldNum(); j++ )
               if ( fieldvisiblenow ( vm.reachableFieldsIndirect.getField ( j )))
                  vm.reachableFieldsIndirect.getField ( j )->a.temp = 1;

            displaymap();

            if ( taste != ct_invvalue ) {
               while ( skeypress ( taste )) {
                  while ( keypress() )
                     r_key();

                  releasetimeslice();
               }
            } else {
               int mb = mouseparams.taste;
               while ( mouseparams.taste == mb && !keypress() )
                  releasetimeslice();

               while ( keypress() )
                  r_key();
            }
            actmap->cleartemps ( 7 );
            displaymap();
         }
      }
      veh->color = oldcolor;
      tcs.restore();
   }
}


void renameUnit()
{
   if ( actmap ) {
      pfield fld = getactfield();
      if ( fld && fld->vehicle && fld->vehicle->getOwner() == actmap->actplayer )
         fld->vehicle->name = editString ( "unit name", fld->vehicle->name );
      if ( fld && fld->building && fld->building->getOwner() == actmap->actplayer )
         fld->building->name = editString ( "building name", fld->building->name );
   }
}


// user actions using the old event system
void execuseraction ( tuseractions action )
{
   switch ( action ) {
      case ua_repainthard  :
      case ua_repaint      :
         repaintdisplay();
         break;

      case ua_help         :
         help(20);
         break;

      case ua_howtostartpbem :
         help(21);
         break;

      case ua_howtocontinuepbem :
         help(22);
         break;

      case ua_dispvehicleimprovement    :
      /*
         displaymessage("Research:\n%s %d \n%s %d \n%s %d \n"
                        "%s %d \n%s %d \n%s %d \n"
                        "%s %d \n%s %d \n%s %d \n"
                        "%s %d",1,
                        cwaffentypen[0], (actmap->player[actmap->actplayer].research.unitimprovement.weapons[0]),
                        cwaffentypen[1], (actmap->player[actmap->actplayer].research.unitimprovement.weapons[1]),
                        cwaffentypen[2], (actmap->player[actmap->actplayer].research.unitimprovement.weapons[2]),
                        cwaffentypen[3], (actmap->player[actmap->actplayer].research.unitimprovement.weapons[3]),
                        cwaffentypen[4], (actmap->player[actmap->actplayer].research.unitimprovement.weapons[4]),
                        cwaffentypen[5], (actmap->player[actmap->actplayer].research.unitimprovement.weapons[5]),
                        cwaffentypen[6], (actmap->player[actmap->actplayer].research.unitimprovement.weapons[6]) ,
                        cwaffentypen[7], (actmap->player[actmap->actplayer].research.unitimprovement.weapons[7])  ,
                        cwaffentypen[10], (actmap->player[actmap->actplayer].research.unitimprovement.weapons[10])     ,
                        "armor",         (actmap->player[actmap->actplayer].research.unitimprovement.armor));
                        */
         break;

      case ua_mainmenu:
         if (choice_dlg("do you really want to close the current game ?","~y~es","~n~o") == 1) {
            delete actmap;
            actmap = NULL;
            throw NoMapLoaded();
         }
         break;

      case ua_mntnc_morefog:
         if (actmap->weather.fog < 255   && maintainencecheck() ) {
            actmap->weather.fog++;
            computeview( actmap );
            displaymessage2("fog intensity set to %d ", actmap->weather.fog);
            displaymap();
         }
         break;

      case ua_mntnc_lessfog:
         if (actmap->weather.fog  && maintainencecheck()) {
            actmap->weather.fog--;
            computeview( actmap );
            displaymessage2("fog intensity set to %d ", actmap->weather.fog);
            displaymap();
         }
         break;

      case ua_mntnc_morewind:
         if ((actmap->weather.windSpeed < 254) &&  maintainencecheck()) {
            actmap->weather.windSpeed+=2;
            displaywindspeed (  );
            updateFieldInfo();
         }
         break;

      case ua_mntnc_lesswind:
         if ((actmap->weather.windSpeed > 1)  && maintainencecheck() ) {
            actmap->weather.windSpeed-=2;
            displaywindspeed (  );
            updateFieldInfo();
         }
         break;

      case ua_mntnc_rotatewind:
         if ( maintainencecheck() ) {
            if (actmap->weather.windDirection < sidenum-1 )
               actmap->weather.windDirection++;
            else
               actmap->weather.windDirection = 0;
            displaymessage2("wind dir set to %d ", actmap->weather.windDirection);
            updateFieldInfo();
            displaymap();
         }
         break;

      case ua_changeresourceview:
         showresources++;
         if ( showresources >= 3 )
            showresources = 0;
         displaymap();
         break;

      case ua_benchgamewov:
         benchMapDisplay();
         // benchgame( 0 );
         break;

      case ua_benchgamewv :
         benchgame( 1 );
         break;

      case ua_viewterraininfo:
         viewterraininfo();
         break;

      case ua_unitweightinfo:
         if ( fieldvisiblenow  ( getactfield() )) {
            Vehicle* eht = getactfield()->vehicle;
            if ( eht && getdiplomaticstatus ( eht->color ) == capeace )
               displaymessage(" weight of unit: \n basic: %d\n+fuel: %d\n+material:%d\n+cargo:%d\n= %d",1 ,eht->typ->weight, eht->getTank().fuel * resourceWeight[Resources::Fuel] / 1000 , eht->getTank().material * resourceWeight[Resources::Material] / 1000, eht->cargo(), eht->weight() );
         }
         break;

      case ua_writemaptopcx :
         writemaptopcx ();
         break;

      case ua_writescreentopcx:
         {
            char* nm = getnextfilenumname ( "screen", "pcx", 0 );
            writepcx ( nm, 0, 0, agmp->resolutionx-1, agmp->resolutiony-1, pal );
            displaymessage2( "screen saved to %s", nm );
         }
         break;

      case ua_startnewsinglelevel:
         startnewsinglelevelfromgame();
         break;

      case ua_changepassword:
         {
            bool success;
            do {
               Password oldpwd = actmap->player[actmap->actplayer].passwordcrc;
               actmap->player[actmap->actplayer].passwordcrc.reset();
               success = enterpassword ( actmap->player[actmap->actplayer].passwordcrc, true, true );
               if ( !success )
                  actmap->player[actmap->actplayer].passwordcrc = oldpwd;
            } while ( actmap->player[actmap->actplayer].passwordcrc.empty() && success && viewtextquery ( 910, "warning", "~e~nter password", "~c~ontinue without password" ) == 0 ); /* enddo */
         }
         break;

      case ua_gamepreferences:
         gamepreferences();
         break;

      case ua_mousepreferences:
         mousepreferences();
         break;

      case ua_bi3preferences:
         bi3preferences();
         break;

      case ua_exitgame:
         /*if (choice_dlg("do you really want to quit ?","~y~es","~n~o") == 1)
            abortgame = 1;
         else
            exitprogram = 0;
            */
            getPGApplication().quit();
         break;

      case ua_newcampaign:
         cursor.hide();
         newcampaign();
         computeview( actmap );
         displaymap();
         cursor.show();
         break;

      case ua_loadgame:
         loadGame();
         break;

      case ua_savegame:
         saveGame( true );
         break;

      case ua_setupalliances:
         setupalliances();
         logtoreplayinfo ( rpl_alliancechange );
         logtoreplayinfo ( rpl_shareviewchange );

         if ( actmap->shareview && actmap->shareview->recalculateview ) {
            logtoreplayinfo ( rpl_shareviewchange );
            computeview( actmap );
            actmap->shareview->recalculateview = 0;
            displaymap();
         }
         updateFieldInfo();
         break;

      case ua_settribute :
         settributepayments ();
         break;

      case ua_giveunitaway:
         if ( actmap && actmap->getgameparameter( cgp_disableUnitTransfer ) == 0 )
            giveunitaway ();
         else
            displaymessage("Sorry, this function has been disabled when starting the map!", 1 );
         break;
      case ua_renameunit:
         renameUnit();
         break;

      case ua_vehicleinfo:
         activefontsettings.font = schriften.smallarial;
         vehicle_information();
         break;

      case ua_researchinfo:
         researchinfo ();
         break;

      case ua_unitstatistics:
         statisticarmies();
         break;

      case ua_buildingstatistics:
         statisticbuildings();
         break;

      case ua_newmessage:
         newmessage();
         break;

      case ua_viewqueuedmessages:
         viewmessages( "queued messages", actmap->unsentmessage, 1, 0 );
         break;

      case ua_viewsentmessages:
         viewmessages( "sent messages", actmap->player[ actmap->actplayer ].sentmessage, 0, 0 );
         break;

      case ua_viewreceivedmessages:
         viewmessages( "received messages", actmap->player[ actmap->actplayer ].oldmessage, 0, 1 );
         break;

      case ua_viewjournal:
         viewjournal();
         break;

      case ua_editjournal:
         editjournal();
         break;

      case ua_viewaboutmessage:
         {
            help(30);
            tviewanytext vat;
            ASCString s = getstartupmessage();

#ifdef _SDL_
            char buf[1000];
            SDL_version compiled;
            SDL_VERSION(&compiled);
            sprintf(buf, "\nCompiled with SDL version: %d.%d.%d\n", compiled.major, compiled.minor, compiled.patch);
            s += buf;

            sprintf(buf, "Linked with SDL version: %d.%d.%d\n", SDL_Linked_Version()->major, SDL_Linked_Version()->minor, SDL_Linked_Version()->patch);
            s += buf;
#endif

            vat.init ( "about", s.c_str() );
            vat.run();
            vat.done();
         }
         break;

      case ua_continuenetworkgame:
         continuenetworkgame();
         displaymap();
         break;

      case ua_toggleunitshading:
         CGameOptions::Instance()->units_gray_after_move = !CGameOptions::Instance()->units_gray_after_move;
         CGameOptions::Instance()->setChanged();
         displaymap();
         while ( mouseparams.taste )
            releasetimeslice();

         if ( CGameOptions::Instance()->units_gray_after_move )
            displaymessage("units that can not move will now be displayed gray", 3);
         else
            displaymessage("units that can not move and cannot shoot will now be displayed gray", 3);
         break;

      case ua_computerturn:
         if ( maintainencecheck() || 1) {
            displaymessage("This function is under development and for programmers only\n"
                           "unpredictable things may happen ...",3 ) ;

            if (choice_dlg("do you really want to start the AI?","~y~es","~n~o") == 1) {

               if ( !actmap->player[ actmap->actplayer ].ai )
                  actmap->player[ actmap->actplayer ].ai = new AI ( actmap, actmap->actplayer );

               savegame ( "aistart.sav" );
               actmap->player[ actmap->actplayer ].ai->run();
            }
         }
         break;
      case ua_setupnetwork:
         if ( actmap->network )
            setupnetwork ( actmap->network );
         else
            displaymessage("This map is not played across a network",3 );
         break;
      case ua_selectgraphicset:
         selectgraphicset();
         break;
      case ua_UnitSetInfo:
         viewUnitSetinfo();
         break;
      case ua_GameParameterInfo:
         showGameParameters();
         break;
      case ua_GameStatus:
         displaymessage ( "Current game time is:\n turn %d , move %d ", 3, actmap->time.turn(), actmap->time.move() );
         break;
      case ua_viewunitweaponrange:
         viewunitweaponrange ( getactfield()->vehicle, ct_invvalue );
         break;

      case ua_viewunitmovementrange:
         viewunitmovementrange ( getactfield()->vehicle, ct_invvalue );
         break;

      case ua_aibench:
         if ( maintainencecheck() || 1 ) {
            if ( !actmap->player[ actmap->actplayer ].ai )
               actmap->player[ actmap->actplayer ].ai = new AI ( actmap, actmap->actplayer );

            if ( AI* ai = dynamic_cast<AI*>( actmap->player[ actmap->actplayer ].ai )) {
               savegame ( "ai-bench-start.sav" );
               ai->run( true );
            }
         }
         break;
      case ua_networksupervisor:
         networksupervisor();
         displaymap();
         break;

      case ua_selectPlayList:
         selectPlayList();
         break;
      case ua_statisticdialog:
         statisticDialog();
         break;

      case ua_showPlayerSpeed:
         showPlayerTime();
         break;
      case  ua_cancelResearch:
         if ( actmap->player[actmap->actplayer].research.activetechnology ) {
            ASCString s = "do you really want to cancel the current research project ?\n";
            // s += strrr ( actmap->player[actmap->actplayer].research.progress );
            // s += " research points will be lost.";
            if (choice_dlg(s.c_str(),"~y~es","~n~o") == 1) {
               actmap->player[actmap->actplayer].research.progress = 0;
               actmap->player[actmap->actplayer].research.activetechnology = NULL;
            }
         } else
            displaymessage("you are not researching anything", 3);
         break;
      case ua_showResearchStatus: {
            ASCString s;
            s += "Current technology:\n";
            if ( actmap->player[actmap->actplayer].research.activetechnology )
               s += actmap->player[actmap->actplayer].research.activetechnology->name;
            else
               s += " - none - ";
            s += "\n\n";

            s += "Research Points: \n";
            s += strrr( actmap->player[actmap->actplayer].research.progress );
            if ( actmap->player[actmap->actplayer].research.activetechnology )
               s += ASCString(" / ") + strrr ( actmap->player[actmap->actplayer].research.activetechnology->researchpoints );
            s += "\n\n";

            s+= "Research Points Plus \n";
            int p = 0;
            for ( Player::BuildingList::iterator i = actmap->player[actmap->actplayer].buildingList.begin(); i != actmap->player[actmap->actplayer].buildingList.end(); ++i )
               p += (*i)->researchpoints;


            s += strrr ( p );

            s += "\n\n";

            s+= "Developed Technologies: \n";
            for ( vector<int>::iterator i = actmap->player[actmap->actplayer].research.developedTechnologies.begin(); i != actmap->player[actmap->actplayer].research.developedTechnologies.end(); ++i ) {
               Technology* t = technologyRepository.getObject_byID( *i );
               if ( t )
                  s += t->name + "\n";
            }


            tviewanytext vat ;
            vat.init ( "Research Status", s.c_str(), 20, -1 , 450, 480 );
            vat.run();
            vat.done();
         }
         break;
      case ua_exportUnitToFile:
         if ( getactfield()->vehicle && getactfield()->vehicle->getOwner() == actmap->actplayer ){
            ASCString s = "do you really want to cut this unit from the game?";
            if (choice_dlg(s.c_str(),"~y~es","~n~o") == 1) {
               Vehicle* veh = getactfield()->vehicle;
               ClipBoard::Instance().clear();
               ClipBoard::Instance().addUnit( veh );

               ASCString filename;
               fileselectsvga(clipboardFileExtension, filename, false);
               if ( !filename.empty() ) {
                  tnfilestream stream ( filename, tnstream::writing );
                  ClipBoard::Instance().write( stream );
                  logtoreplayinfo ( rpl_cutFromGame, veh->networkid );
                  veh->prepareForCleanRemove();
                  delete veh;
                  computeview( actmap );
                  displaymap();
               }
            }
         }
         break;
      default:
         break;
   }
}


// user actions using the new event system
void execuseraction2 ( tuseractions action )
{
   switch ( action ) {
   
      case ua_soundDialog:
         soundSettings();
         break;
      case ua_reloadDlgTheme:
             getPGApplication().reloadTheme();
             // soundSettings();
         break;
      case ua_viewButtonPanel:  mainScreenWidget->spawnPanel( MainScreenWidget::ButtonPanel );
         break;
      case ua_viewWindPanel:     mainScreenWidget->spawnPanel( MainScreenWidget::WindInfo );
         break;
      case ua_clearImageCache:  IconRepository::clear();
         break;   
      case ua_viewUnitInfoPanel: mainScreenWidget->spawnPanel( MainScreenWidget::UnitInfo );
      default:
         break;
   }

}


void mainloopgeneralkeycheck ( tkey& ch )
{
   int keyprn;
   getkeysyms ( &ch, &keyprn );

   movecursor(ch);
   actgui->checkforkey ( ch, keyprn );
}




void mainloopgeneralmousecheck ( void )
{
   if ( exitprogram )
      execuseraction ( ua_exitgame );

   actgui->checkformouse();


   if ( lastdisplayedmessageticker + messagedisplaytime < ticker )
      displaymessage2("");


   {
      int oldx = actmap->xpos;
      int oldy = actmap->ypos;
      checkformousescrolling();
      if ( oldx != actmap->xpos || oldy != actmap->ypos )
         updateFieldInfo();
   }

   if ( onlinehelp )
      onlinehelp->checkforhelp();
/*
   if ( onlinehelpwind && !CGameOptions::Instance()->smallmapactive )
      onlinehelpwind->checkforhelp();
      */
}


bool execUserActionI  (PG_PopupMenu::MenuItem* menuItem )
{
   getPGApplication().enableLegacyEventHandling ( true );
   execuseraction( tuseractions( menuItem->getId() ) );
   getPGApplication().enableLegacyEventHandling ( false );
   execuseraction2( tuseractions( menuItem->getId() ));
   return true;
}


Menu::~Menu()
{
/*
   for ( Categories::iterator i = categories.begin(); i != categories.end(); ++i )
      delete *i;
*/      
      
}

void Menu::addfield( const char* name )
{
   ASCString s = name;
   while ( s.find ( "~") != ASCString::npos )
      s.erase( s.find( "~"),1 );
      
   currentMenu = new PG_PopupMenu( NULL, -1, -1, "" );
   categories.push_back ( currentMenu );
   Add ( s, currentMenu );
   currentMenu->sigSelectMenuItem.connect( SigC::slot( execUserActionI));

}

void Menu::addbutton( const char* name, int id )
{
   ASCString s = name;
   while ( s.find ( "~") != ASCString::npos )
      s.erase( s.find( "~"),1 );
      
   currentMenu->addMenuItem( s, id );
}


void Menu::setup()
{
   addfield ( "Glo~b~al" );
   addbutton ( "toggle ~R~esourceview�1", ua_changeresourceview );
   addbutton ( "toggle unit shading�2", ua_toggleunitshading );
   currentMenu->addSeparator();
   addbutton ( "~O~ptions", ua_gamepreferences );
   addbutton ( "~M~ouse options", ua_mousepreferences );
   addbutton ( "~S~ound options", ua_soundDialog );
   currentMenu->addSeparator();
   addbutton ( "E~x~it�ctrl-x", ua_exitgame );


   addfield ("~G~ame");
   addbutton ( "New ~C~ampaign", ua_newcampaign);
   addbutton ( "~N~ew single Level�ctrl-n", ua_startnewsinglelevel );
   currentMenu->addSeparator();
   addbutton ( "~L~oad game�ctrl-l", ua_loadgame );
   addbutton ( "~S~ave game�ctrl-s", ua_savegame );
   currentMenu->addSeparator();
   addbutton ( "Continue network game�F3", ua_continuenetworkgame);
   addbutton ( "setup Net~w~ork", ua_setupnetwork );
   addbutton ( "Change Passw~o~rd", ua_changepassword );
   addbutton ( "supervise network game", ua_networksupervisor );
   currentMenu->addSeparator();
   addbutton ( "~P~layers + Alliances", ua_setupalliances);
   addbutton ( "transfer ~U~nit control", ua_giveunitaway );
   addbutton ( "~r~ename unit/building", ua_renameunit );
   addbutton ( "~T~ransfer resources", ua_settribute);
   addbutton ( "~C~ancel Research", ua_cancelResearch );

   addfield ( "~I~nfo" );
   addbutton ( "~V~ehicle types", ua_vehicleinfo );
   addbutton ( "Unit ~w~eapon range�3", ua_viewunitweaponrange );
   addbutton ( "Unit ~m~ovement range�4", ua_viewunitmovementrange );
   addbutton ( "~G~ame Time�5", ua_GameStatus );
   addbutton ( "unit ~S~et information�6", ua_UnitSetInfo );
   addbutton ( "~T~errain�7", ua_viewterraininfo );
   addbutton ( "~U~nit weight�8", ua_unitweightinfo );
   addbutton ( "show ~P~ipeline net�9", ua_viewPipeNet );
   currentMenu->addSeparator();
   addbutton ( "~R~esearch", ua_researchinfo );
   addbutton ( "~P~lay time", ua_showPlayerSpeed );
   // addbutton ( "~R~esearch status", ua_showResearchStatus );

   // addbutton ( "vehicle ~I~mprovement�F7", ua_dispvehicleimprovement);
   // addbutton ( "show game ~P~arameters", ua_GameParameterInfo );


   addfield ( "~S~tatistics" );
   addbutton ( "~U~nits", ua_unitstatistics );
   addbutton ( "~B~uildings", ua_buildingstatistics );
//   addbutton ( "~R~esources ", ua_statisticdialog );
   // addbutton ( "seperator");
   // addbutton ( "~H~istory");

   addfield ( "~M~essage");
   addbutton ( "~n~ew message", ua_newmessage );
   addbutton ( "view ~q~ueued messages", ua_viewqueuedmessages );
   addbutton ( "view ~s~end messages", ua_viewsentmessages );
   addbutton ( "view ~r~eceived messages", ua_viewreceivedmessages);
   currentMenu->addSeparator();
   addbutton ( "view ~j~ournal", ua_viewjournal );
   addbutton ( "~a~ppend to journal", ua_editjournal );

   addfield ( "~T~ools" );
   addbutton ( "save ~M~ap as PCX�9", ua_writemaptopcx );
   addbutton ( "save ~S~creen as PCX", ua_writescreentopcx );
   addbutton ( "benchmark without view calc", ua_benchgamewov );
   addbutton ( "benchmark with view calc", ua_benchgamewv);
   addbutton ( "compiler benchmark (AI)", ua_aibench );
   // addbutton ( "test memory integrity", ua_heapcheck );
   currentMenu->addSeparator();
   addbutton ( "select graphic set", ua_selectgraphicset );

   addfield ( "~V~iew" );
   addbutton ( "Button Panel", ua_viewButtonPanel );
   addbutton ( "Wind Panel", ua_viewWindPanel );
   addbutton ( "Unit Info Panel", ua_viewUnitInfoPanel );
   currentMenu->addSeparator();
   addbutton ( "clear image cache", ua_clearImageCache );
   addbutton ( "reload dialog theme", ua_reloadDlgTheme );
   
   
   addfield ( "~H~elp" );
   addbutton ( "HowTo ~S~tart email games", ua_howtostartpbem );
   addbutton ( "HowTo ~C~ontinue email games", ua_howtocontinuepbem );
   currentMenu->addSeparator();
   addbutton ( "~K~eys", ua_help );

   addbutton ( "~A~bout", ua_viewaboutmessage );
}


Menu::Menu ( PG_Widget *parent, const PG_Rect &rect)
    : PG_MenuBar( parent, rect, "MenuBar"),
      currentMenu(NULL)
{
   setup();
   
}  





pfield        getactfield(void)
{
   return actmap->getField( actmap->player[actmap->actplayer].cursorPos ); 
} 



MainScreenWidget::MainScreenWidget( PG_Application& application )
              : PG_Widget(NULL, PG_Rect ( 0, 0, app.GetScreen()->w, app.GetScreen()->h ), false),
              app ( application ) 
{
   mapDisplay = new MapDisplayPG( this, PG_Rect(20,20,Width() - 200, Height() - 40));
   menu = new Menu(this, PG_Rect(0,0,Width(),20));

   SetID( 1 );
      
   spawnPanel ( ButtonPanel );
   spawnPanel ( WindInfo );
   spawnPanel ( UnitInfo );
}


void MainScreenWidget::spawnPanel ( Panels panel )
{
   if ( panel == ButtonPanel ) {
      guiHost = new NewGuiHost( this, PG_Rect(Width()-180, 20, 170, 300));
      guiHost->pushIconHandler( &GuiFunctions::primaryGuiIcons );
      guiHost->Show();
   }
   if ( panel == WindInfo ) {
      WindInfoPanel* wi = new WindInfoPanel( this, PG_Rect(Width()-180, 320, 170, 80));
      wi->Show();
   }
   if ( panel == UnitInfo ) {
      UnitInfoPanel* ui = new UnitInfoPanel( this, PG_Rect(Width()-180, 400, 170, 220));
      ui->Show();
   }
}


void  mainloop2()
{
   mainScreenWidget = new MainScreenWidget( getPGApplication());
   mainScreenWidget->Show();
   
   getPGApplication().Run();
}

void  mainloop ( void )
{
   tkey ch;

   do {
      viewunreadmessages();
      activefontsettings.background=0;
      activefontsettings.length=50;
      activefontsettings.color=14;
      if (keypress()) {

         mainloopgeneralkeycheck ( ch );

         switch (ch) {

#ifndef NEWKEYB
            case 'R':
               execuseraction ( ua_repainthard );
               break;
#else
            case ct_shp + ct_r:
               execuseraction ( ua_repainthard );
               break;
#endif

            case ct_stp + ct_l:
               execuseraction ( ua_loadgame );
               break;

            case ct_stp + ct_s:
               execuseraction ( ua_savegame );
               break;


            case ct_stp + ct_n:
               execuseraction ( ua_startnewsinglelevel );
               break;

            case ct_stp + ct_f12:
               execuseraction ( ua_exportUnitToFile );
               break;

            case ct_r:
               execuseraction ( ua_repaint );
               break;

            case ct_f1:
               execuseraction ( ua_help );
               break;

            case ct_f2:
               execuseraction ( ua_mainmenu );
               break;

            case ct_f3:
               execuseraction ( ua_continuenetworkgame );
               break;

            case ct_f4:
               execuseraction ( ua_computerturn );
               break;

            case ct_f8:
               {
                  int color = actmap->actplayer;
                  for ( int p = 0; p < 8; p++ )
                     if ( actmap->player[p].stat == Player::computer && actmap->player[p].exist() )
                        color = p;

                  if ( actmap->player[color].ai ) {
                     AI* ai = (AI*) actmap->player[color].ai;
                     ai->showFieldInformation ( getxpos(), getypos() );
                  }
               }
               break;

            case ct_f11: {
            // computeview ( actmap );
            }
            break;

            case ct_1:
               execuseraction ( ua_changeresourceview );
               break;

            case ct_2:
               execuseraction ( ua_toggleunitshading );
               break;

            case ct_3:
               viewunitweaponrange ( getactfield()->vehicle, ct_3 );
               break;

            case ct_4:
               viewunitmovementrange ( getactfield()->vehicle, ct_4 );
               break;

            case ct_5:
               execuseraction ( ua_GameStatus );
               break;

            case ct_6:
               execuseraction ( ua_UnitSetInfo );
               break;

            case ct_7:
               execuseraction ( ua_viewterraininfo );
               break;

            case ct_8:
               execuseraction ( ua_unitweightinfo );
               break;

            case ct_9:
               viewPipeNet ( ct_9 );
               break;

            case ct_0: execuseraction( ua_writescreentopcx );
               break;

            case ct_x + ct_stp:
               execuseraction ( ua_exitgame );
               break;
         }

      } else
         ch = ct_invvalue;


      mainloopgeneralmousecheck ( );

      /************************************************************************************************/
      /*        Pulldown Men?                                                                       . */
      /************************************************************************************************/

      while ( actmap->player[ actmap->actplayer ].queuedEvents )
         checkevents( &getDefaultMapDisplay() );

      checktimedevents( &getDefaultMapDisplay() );

      checkforvictory();

      releasetimeslice();

   }  while ( true );

}



pfont load_font ( const char* name )
{
   tnfilestream stream ( name , tnstream::reading );
   return loadfont ( &stream );
}

const char* progressbarfilename = "progress.6mn";



void loaddata( int resolx, int resoly, const char *gameToLoad=NULL )
{
   actprogressbar = new tprogressbar;
   if ( actprogressbar ) {
      tfindfile ff ( progressbarfilename );
      if ( !ff.getnextname().empty() ) {
         tnfilestream strm ( progressbarfilename, tnstream::reading );
         actprogressbar->start ( 255, 0, agmp->resolutiony-3, agmp->resolutionx-1, agmp->resolutiony-1, &strm );
      } else {
         actprogressbar->start ( 255, 0, agmp->resolutiony-3, agmp->resolutionx-1, agmp->resolutiony-1, NULL );
      }
   }

   schriften.smallarial = load_font("smalaril.fnt");
   schriften.large = load_font("usablack.fnt");
   schriften.arial8 = load_font("arial8.fnt");
   schriften.smallsystem = load_font("msystem.fnt");
   schriften.guifont = load_font("gui.fnt");
   schriften.guicolfont = load_font("guicol.fnt");
   schriften.monogui = load_font("monogui.fnt");
  #if 0
   TTF_Init();
    tnfilestream fs( "C:\\Programme\\ascdev\\asc\\data\\dialog\\asc\\font.ttf", tnstream::reading );
    std::cout << fs.getLocation() << " size:" << fs.getSize() << std::endl; //Ausgabe stimmt
    SDL_RWops* ops = SDL_RWFromStream( &fs );
    if(!ops){//Ist false
       std::cout << "loading failed" << std::endl;
    }
    TTF_Font* myFont = TTF_OpenFontIndexRW(ops, 0, 14,0); //klappt nicht
    //SDL_RWclose(ops);
    /*std::FILE* filep = fopen(file.c_str(), "r");
    SDL_RWops* ops2 = SDL_RWFromFP( filep, 0 );
    myFont = TTF_OpenFontRW(ops2, 0, size);
    */
    //myFont = TTF_OpenFont(file.c_str(), size);
    if(0 == myFont){ //Wird betreten
       printf("TTF_OpenFontRW: %s\n", TTF_GetError());
    }
#endif

   GraphicSetManager::Instance().loadData();

   activefontsettings.markfont = schriften.guicolfont;
   shrinkfont ( schriften.guifont, -1 );
   shrinkfont ( schriften.guicolfont, -1 );
   shrinkfont ( schriften.monogui, -1 );
   pulldownfont = schriften.smallarial ;

   if ( actprogressbar ) actprogressbar->startgroup();

   SoundList::init();

   if ( actprogressbar ) actprogressbar->startgroup();

   loadMoreData();

   if ( actprogressbar ) actprogressbar->startgroup();

   registerDataLoader ( new PlayListLoader() );
   registerDataLoader ( new BI3TranslationTableLoader() );

   loadguipictures();
   if ( actprogressbar ) actprogressbar->startgroup();
   loadAllData();

   if ( actprogressbar ) actprogressbar->startgroup();
   loadUnitSets();

   if ( actprogressbar ) actprogressbar->startgroup();

   cursor.init();
   selectbuildinggui.init( resolx, resoly );
   selectobjectcontainergui.init( resolx, resoly );
   selectvehiclecontainergui.init( resolx, resoly );

   if ( actprogressbar ) actprogressbar->startgroup();

   loadStartupMap( gameToLoad );

   if ( actprogressbar ) actprogressbar->startgroup();

   displayLogMessage ( 6, "done\n" );

   if ( actprogressbar ) actprogressbar->startgroup();

   if ( actprogressbar ) {
      actprogressbar->end();
      try {
         tnfilestream strm ( progressbarfilename, tnstream::writing );
         actprogressbar->writetostream( &strm );
      } /* endtry */
      catch ( tfileerror ) { } /* endcatch */

      delete actprogressbar;
      actprogressbar = NULL;
   }
   
   registerGuiFunctions( GuiFunctions::primaryGuiIcons );
}





//! A Paragui widget that fills the whole screen and redraws it whenever Paragui wants to it.


void runmainmenu ( void )
{
   MainMenuPullDown pd;
   pd.init();
   backgroundpict.paint();
   pd.baron();
   // loadFullscreenImage ( "title.jpg" );

   do {
      tkey ch = ct_invvalue;
      if (keypress()) {
         ch = r_key();

         switch (ch) {
            case ct_f3:
               execuseraction ( ua_continuenetworkgame );
               break;
            case 'R':
               execuseraction ( ua_repainthard );
               break;
            case ct_stp + ct_l:
               execuseraction ( ua_loadgame );
               break;
            case ct_stp + ct_n:
               execuseraction ( ua_startnewsinglelevel );
               break;
            case ct_x + ct_stp:
               execuseraction ( ua_exitgame );
               break;
         };
      }

      pd.key = ch;
      pd.checkpulldown();

      if (pd.action2execute >= 0 ) {
         tuseractions ua = (tuseractions) pd.action2execute;
         pd.action2execute = -1;
         execuseraction ( ua );
         pd.redraw();
      }

      releasetimeslice();
   } while ( !actmap  ); /* enddo */

}



struct GameThreadParams
{
   ASCString filename;
   ASC_PG_App& application;
   GameThreadParams( ASC_PG_App& app ) : application ( app ){};
};

int gamethread ( void* data )
{
   GameThreadParams* gtp = (GameThreadParams*) data;

   loadpalette();
   initMapDisplay( );

   int resolx = agmp->resolutionx;
   int resoly = agmp->resolutiony;
   gui.init ( resolx, resoly );
   virtualscreenbuf.init();

   try {
      int fs = loadFullscreenImage ( "title.jpg" );
      if ( !fs ) {
         tnfilestream stream ( "logo640.pcx", tnstream::reading );
         loadpcxxy( &stream, (hgmp->resolutionx - 640)/2, (hgmp->resolutiony-35)/2, 1 );
      }
      loaddata( resolx, resoly, gtp->filename.c_str() );
      if ( fs )
         closeFullscreenImage ();

   }
   catch ( ParsingError err ) {
      displaymessage ( "Error parsing text file " + err.getMessage(), 2 );
   }
   catch ( tfileerror err ) {
      displaymessage ( "Error loading file " + err.getFileName(), 2 );
   }
   catch ( ASCexception ) {
      displaymessage ( "loading of game failed", 2 );
   }
   catch ( ... ) {
      displaymessage ( "caught undefined exception", 2 );
   }

   displayLogMessage ( 5, "loaddata completed successfully.\n" );
   setvgapalette256(pal);

   addmouseproc ( &mousescrollproc );

   displayLogMessage ( 5, "starting music..." );
   startMusic();
   displayLogMessage ( 5, " done \n" );

   onlinehelp = new tsgonlinemousehelp;
   onlinehelpwind = new tsgonlinemousehelpwind;

   gameStartupComplete = true;

   repaintDisplay.connect( repaintMap );
   
   displayLogMessage ( 5, "entering outer main loop.\n" );
   do {
      try {
         if ( !actmap || actmap->xsize <= 0 || actmap->ysize <= 0 ) {
            displayLogMessage ( 8, "gamethread :: starting main menu.\n" );
            runmainmenu();
         } else {
            if ( actmap->actplayer == -1 ) {
               displayLogMessage ( 8, "gamethread :: performing next_turn..." );
               next_turn();
               displayLogMessage ( 8, "done.\n" );
            }

            displayLogMessage ( 8, "gamethread :: Painting background pict..." );
            // backgroundpict.paint();

            if ( !gtp->filename.empty() && patimat ( tournamentextension, gtp->filename.c_str() ) ) {
               displayLogMessage ( 5, "Initializing network game..." );
               initNetworkGame ( );
               displayLogMessage ( 5, "done\n" );
            }

            displayLogMessage ( 8, "gamethread :: displaying map..." );
            // displaymap();
            displayLogMessage ( 8, "done.\n" );
            // cursor.show();

            moveparams.movestatus = 0;

            displayLogMessage ( 8, "gamethread :: painting gui icons..." );
            // actgui->painticons();
            displayLogMessage ( 8, "done.\n" );
            mousevisible(true);

            updateFieldInfo();

            displayLogMessage ( 5, "entering inner main loop.\n" );
            mainloop2();
            mousevisible ( false );
         }
      } /* endtry */
      catch ( NoMapLoaded ) { } /* endcatch */
      catch ( LoadNextMap lnm ) {
         if ( actmap->campaign ) {
            startnextcampaignmap( lnm.id );
         } else {
           viewtext2(904);
           if (choice_dlg("Do you want to continue playing ?","~y~es","~n~o") == 2) {
              delete actmap;
              actmap = NULL;
           } else {
              actmap->continueplaying = 1;
              if ( actmap->replayinfo ) {
                 delete actmap->replayinfo;
                 actmap->replayinfo = NULL;
              }
           }
         }
      }
   } while ( false );
   return 0;
}


// including the command line parser, which is generated by genparse
#include "clparser/asc.cpp"

int main(int argc, char *argv[] )
{
   Cmdline* cl = NULL;
   auto_ptr<Cmdline> apcl ( cl );
   try {
      cl = new Cmdline ( argc, argv );
   } catch ( string s ) {
      cerr << s.c_str();
      exit(1);
   }

   /*
   if ( cl->next_param() < argc ) {
      cerr << "invalid command line parameter\n";
      exit(1);
   }*/

   if ( cl->v() ) {
      ASCString msg = getstartupmessage();
      printf( msg.c_str() );
      exit(0);
   }

   if ( cl->w() )
      fullscreen = SDL_FALSE;

   if ( cl->f() )
      fullscreen = SDL_TRUE;

   verbosity = cl->r();

   displayLogMessage( 1, getstartupmessage() );

   mapborderpainter = &backgroundpict;

   initFileIO( cl->c().c_str() );  // passing the filename from the command line options

   try {
      checkDataVersion();
      // check_bi3_dir ();
   } catch ( tfileerror err ) {
      displaymessage ( "unable to access file %s \n", 2, err.getFileName().c_str() );
   }
   catch ( ... ) {
      displaymessage ( "loading of game failed during pre graphic initializing", 2 );
   }

   if ( CGameOptions::Instance()->forceWindowedMode && !cl->f() )  // cl->f == force fullscreen command line param
      fullscreen = SDL_FALSE;

   SDLmm::Surface* icon = NULL;
   try {
      tnfilestream iconl ( "icon_asc.gif", tnstream::reading );
      SDL_Surface *icn = IMG_LoadGIF_RW( SDL_RWFromStream ( &iconl ));
      SDL_SetColorKey(icn, SDL_SRCCOLORKEY, *((Uint8 *)icn->pixels));
      icon = new SDLmm::Surface ( icn );
   } catch ( ... ) {}


   int xr = 800;
   int yr = 600;
   // determining the graphics resolution
   if ( CGameOptions::Instance()->xresolution != 800 )
      xr = CGameOptions::Instance()->xresolution;
   if ( cl->x() != 800 )
      xr = cl->x();

   if ( CGameOptions::Instance()->yresolution != 600 )
      yr = CGameOptions::Instance()->yresolution;
   if ( cl->y() != 600 )
      yr = cl->y();


   SoundSystem soundSystem ( CGameOptions::Instance()->sound.muteEffects, CGameOptions::Instance()->sound.muteMusic, cl->q() || CGameOptions::Instance()->sound.off );

   soundSystem.setMusicVolume ( CGameOptions::Instance()->sound.musicVolume );
   soundSystem.setEffectVolume ( CGameOptions::Instance()->sound.soundVolume );

   

   ASC_PG_App app ( "asc2_dlg" );

  
   int flags = SDL_SWSURFACE;
   if ( fullscreen )
      flags |= SDL_FULLSCREEN;


   SDL_WM_SetIcon( icon->GetSurface(), NULL );
   app.InitScreen( xr, yr, 32, flags);
  
//   initASCGraphicSubsystem ( app.GetScreen(), icon );

   
   setWindowCaption ( "Advanced Strategic Command" );
      
   GameThreadParams gtp ( app );
   gtp.filename = cl->l();

   if ( cl->next_param() < argc )
      for ( int i = cl->next_param(); i < argc; i++ )
         gtp.filename = argv[i];


   {
      int w;
      tnfilestream stream ("mausi.raw", tnstream::reading);
      stream.readrlepict(   &icons.mousepointer, false, &w );
   }


   try {
      // this starts the gamethread procedure, whichs will run the entire game
      initializeEventHandling ( gamethread, &gtp, icons.mousepointer );
   }
   catch ( bad_alloc ) {
      fatalError ("Out of memory");
   }

   delete actmap;
   actmap = NULL;

   closegraphics();

   writegameoptions ( );

   delete onlinehelp;
   onlinehelp = NULL;

   delete onlinehelpwind;
   onlinehelpwind = NULL;

#ifdef MEMCHK
   verifyallblocks();
#endif
   return(0);
}

