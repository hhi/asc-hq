/*! \file dashboard.cpp
    \brief The box displaying unit information
*/


/***************************************************************************
                          dashboard.cpp  -  description
                             -------------------
    begin                : Sat Jan 27 2001
    copyright            : (C) 2001 by Martin Bickel
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

 #include "sigc++/adaptors/retype.h"

#include "dashboard.h"
#include "graphics/blitter.h"
#include "graphics/drawing.h"
#include "gamemap.h"
#include "iconrepository.h"
#include "spfst.h"
#include "pgimage.h"
#include "textfiletags.h"
#include "mapdisplay.h"
#include "dialogs/unitinfodialog.h"
#include "dialogs/vehicletypeselector.h"
#include "gameoptions.h"
#include "graphics/ColorTransform_PlayerColor.h"
#include "dialogs/unitnaming.h"
#include "actions/actioncontainer.h"
#include "widgets/textrenderer.h"

#include "sg.h"
#include "spfst-legacy.h"
#include "actions/renamecontainercommand.h"

class WeaponInfoLine;

class WeaponInfoPanel : public Panel {
        int weaponCount;
        static ASCString name;

        vector<WeaponInfoLine*> weaponInfoLines;

     protected:
        bool onClick ( PG_MessageObject* obj, const SDL_MouseButtonEvent* event );
        void painter ( const PG_Rect &src, const ASCString& name, const PG_Rect &dst);

	     bool eventMouseMotion(const SDL_MouseMotionEvent* motion);

     public:
        WeaponInfoPanel (PG_Widget *parent, const Vehicle* veh, const VehicleType* vt ) ;
        void showWeapon( const SingleWeapon* weap = NULL );

        // virtual bool   eventMouseButtonDown (const SDL_MouseButtonEvent *button);
        bool   eventMouseButtonUp (const SDL_MouseButtonEvent *button);
        
        static const ASCString& WIP_Name();
        // void eval();
};



class ExperienceOverview : public PG_Widget {
      static const int columns = 4;
   protected:
      bool 	eventMouseButtonUp (const SDL_MouseButtonEvent *button)
      {
         QuitModal();
         return true;
      };

      static PG_Rect getSize( const PG_Point& pos )
      {
         const Surface& s = IconRepository::getIcon("experience0.png");
         return PG_Rect( pos.x, pos.y, columns * s.w(), (maxunitexperience + columns-1)/columns * s.h() );
      }
      
   public:
      ExperienceOverview( const PG_Point& pos, int exp = -1 ) : PG_Widget( NULL, getSize(pos), true )
      {
         // PG_Application::GetApp()->sigMouseButtonUp.connect( sigc::mem_fun( *this, &ExperienceOverview::QuitModal ));
      }


      int RunModal()
      {
         SetCapture();
         return PG_Widget::RunModal();
      }

      void 	eventDraw (SDL_Surface *surface, const PG_Rect &rect) 
      {
         const Surface& s = IconRepository::getIcon("experience0.png");
         int width = s.w();
         int height = s.h();

         Surface s2 = Surface::Wrap( surface );
         for ( int i = 0; i <= maxunitexperience; ++i ) 
            s2.Blit( IconRepository::getIcon("experience" + ASCString::toString(i) + ".png"), SPoint( i % columns * width , i/columns*height) );
      }
};

DashboardPanel::DashboardPanel ( PG_Widget *parent, const PG_Rect &r, const ASCString& panelName_, bool loadTheme = true )
   :LayoutablePanel ( parent, r, panelName_, loadTheme ), veh(NULL), bld(NULL)
{
   updateFieldInfo.connect ( sigc::mem_fun( *this, &DashboardPanel::eval ));
   registerSpecialDisplay( "windarrow" );

   registerSpecialDisplay( "unitexp" );
   registerSpecialDisplay( "unit_level" );
   registerSpecialDisplay( "unit_pic" );
   for ( int i = 0; i < 10; ++i)
      registerSpecialDisplay( "symbol_weapon" + ASCString::toString(i) );
   registerSpecialDisplay( "showplayercolor0" );
   registerSpecialDisplay( "showplayercolor1" );
   registerSpecialDisplay( "field_weather" );

   ContainerBase::anyContainerDestroyed.connect( sigc::mem_fun( *this, &DashboardPanel::containerDeleted ));
   
   GameMap::sigMapDeletion.connect( sigc::mem_fun( *this, &DashboardPanel::reset ));

   PG_Widget* w = parent->FindChild( "unitexp", true );
   if ( w )
      w->sigMouseButtonDown.connect( sigc::hide( sigc::hide( sigc::mem_fun( *this, &DashboardPanel::viewExperienceOverview ))));
};

bool DashboardPanel::viewExperienceOverview()
{
   PG_Widget* w = GetParent()->FindChild( "unitexp", true );
   if ( w ) {
      ExperienceOverview eo(PG_Point( w->my_xpos, w->my_ypos ));
      eo.Show();
      eo.RunModal();
      return true;
   } else
      return false;
}


void DashboardPanel::containerDeleted( ContainerBase* c )
{
   if ( c == veh )
      veh = NULL;
   
   if ( c == bld )
      bld = NULL;
}


bool DashboardPanel::containerRenamed( PG_LineEdit* lineEdit )
{
   if ( veh ) {
      if ( veh->getMap()->actplayer == veh->getOwner() )
         veh->name = lineEdit->GetText();
      else
         lineEdit->SetText( veh->name );
   }
   
   if ( bld ) {
      if ( bld->getMap()->actplayer == bld->getOwner() )
         bld->name = lineEdit->GetText();
      else
         lineEdit->SetText( bld->name );
   }

   return true;
}

void DashboardPanel::reset(GameMap& map)
{
   if ( veh && veh->getMap() == &map )
      veh = NULL;
   
   if ( bld && bld->getMap() == &map )
      bld = NULL;
}



void DashboardPanel::registerSpecialDisplay( const ASCString& name )
{
   SpecialDisplayWidget* sdw = dynamic_cast<SpecialDisplayWidget*>( FindChild( name, true ) );
   if ( sdw )
     sdw->display.connect( sigc::mem_fun( *this, &DashboardPanel::painter ));
}


void DashboardPanel::painter ( const PG_Rect &src, const ASCString& name, const PG_Rect &dst)
{
   if ( !actmap || actmap->actplayer < 0 || actmap->actplayer > 8  )
      return;

   Surface screen = Surface::Wrap( PG_Application::GetScreen() );

   if ( name == "windarrow" ) {
      if ( actmap && actmap->weather.windSpeed > 0 ) {
         MegaBlitter<4,colorDepth,ColorTransform_None, ColorMerger_AlphaOverwrite, SourcePixelSelector_DirectRotation> blitter;
         blitter.setAngle( (6 - actmap->weather.windDirection) * (360 /6));
         blitter.blit ( IconRepository::getIcon("wind-arrow.png"), screen, SPoint(dst.x, dst.y) );
      }
      return;
   }

   if( name == "showplayercolor0" || name == "showplayercolor1" ) {
      MegaBlitter<4,4,ColorTransform_PlayerTrueCol,ColorMerger_PlainOverwrite> blitter;
      blitter.setColor( actmap->player[actmap->actplayer].getColor() );
      // blitter.setPlayer(actmap->actplayer);
      blitter.blit( IconRepository::getIcon("show_playercolor.png"), screen, SPoint(dst.x, dst.y));
      return;
   }
   
   
   if ( name == "field_weather" ) {
      MapCoordinate mc = actmap->getCursor();
      if ( actmap && mc.valid() && fieldvisiblenow( actmap->getField(mc), actmap->getPlayerView() ) ) {
         MegaBlitter<4,colorDepth,ColorTransform_None, ColorMerger_AlphaOverwrite> blitter;

         static const char* weathernames[] = {"terrain_weather_dry.png",
                                              "terrain_weather_lightrain.png",
                                              "terrain_weather_heavyrain.png",
                                              "terrain_weather_lightsnow.png",
                                              "terrain_weather_heavysnow.png",
                                              "terrain_weather_ice.png" };

         blitter.blit ( IconRepository::getIcon(weathernames[actmap->getField(mc)->getWeather()]), screen, SPoint(dst.x, dst.y) );
      }
      return;
   }




   
   if ( veh && !fieldvisiblenow( veh->getMap()->getField( veh->getPosition() ), veh->getMap()->getPlayerView() ))
      return;
   


      if ( name == "unitexp" ) {
         int experience = 0;
         if ( veh )
            experience = veh->experience;

         screen.Blit( IconRepository::getIcon("experience" + ASCString::toString(experience) + ".png"), SPoint(dst.x, dst.y) );
      }

      if ( name == "unit_level" ) {
         int height1 = 0;
         int height2 = 0;
         int player = actmap->actplayer;
         if ( veh ) {
            height1 = veh->height;
            height2 = veh->typ->height;
            player = veh->getOwner();
         }

         for ( int i = 0; i < 8; ++i ) {
            if ( height1 & (1 << i )) {
               MegaBlitter<4,4,ColorTransform_PlayerTrueCol,ColorMerger_PlainOverwrite> blitter;
               blitter.setColor( actmap->player[player].getColor() );
               // blitter.setPlayer( player );
               blitter.blit( IconRepository::getIcon("height-b" + ASCString::toString(i) + ".png"), screen, SPoint(dst.x, dst.y + (7-i) * 13));
            } else
               if ( height2 & (1 << i ))
                  screen.Blit( IconRepository::getIcon("height-a" + ASCString::toString(i) + ".png"), SPoint(dst.x, dst.y + (7-i) * 13 ) );

         }
      }

      if ( name == "unit_pic" ) {
         if ( veh )
           veh->typ->paint( screen, SPoint( dst.x, dst.y ), veh->getOwningPlayer().getPlayerColor() );
      }

      if ( veh ) {
         int pos = 0;
         for ( int i = 0; i < veh->typ->weapons.count; ++i) {
            if ( !veh->typ->weapons.weapon[i].service() && pos < 10 ) {
               if ( name == "symbol_weapon" + ASCString::toString(pos) )
                  screen.Blit( IconRepository::getIcon(SingleWeapon::getIconFileName( veh->typ->weapons.weapon[i].getScalarWeaponType()) + "-small.png"), SPoint(dst.x, dst.y));

               ++pos;
             }
          }
      }

}


void DashboardPanel::eval()
{
   if ( !actmap || actmap->actplayer < 0 )
      return;


   MapCoordinate mc = actmap->player[actmap->actplayer].cursorPos;
   MapField* fld = actmap->getField(mc);

   Vehicle* veh = fld? fld->vehicle : NULL;

   BulkGraphicUpdates bgu( this );

   setBargraphValue( "winddisplay", float(actmap->weather.windSpeed ) / 255  );

   setLabelText( "windspeed", actmap->weather.windSpeed );

   if ( fld && mc.valid() && fieldvisiblenow( fld, actmap->getPlayerView() )) {
      setLabelText( "terrain_harbour", fld->bdt.test(cbharbour) ? "YES" : "NO" );
      setLabelText( "terrain_pipe", fld->bdt.test(cbpipeline) || fld->building ? "YES" : "NO" );

      setLabelText( "terrain_defencebonus", fld->getdefensebonus() );
      setLabelText( "terrain_attackbonus", fld->getattackbonus() );
      setLabelText( "terrain_jam", fld->getjamming() );
      setLabelText( "terrain_name", fld->typ->terraintype->name );

      int unitspeed;
      if ( veh )
         unitspeed = getmaxwindspeedforunit ( veh );
      else
         unitspeed = maxint;

       int windspeed = actmap->weather.windSpeed*maxwindspeed ;
       if ( unitspeed < 255*256 ) {
          if ( windspeed > unitspeed*9/10 )
             setBarGraphColor( "winddisplay", 0xff0000  );
          else
             if ( windspeed > unitspeed*66/100 )
                setBarGraphColor( "winddisplay", 0xffff00  );
             else
                setBarGraphColor( "winddisplay", 0x00ff00  );
       } else
          setBarGraphColor( "winddisplay", 0x00ff00  );

   } else {
      setLabelText( "terrain_harbour", "" );
      setLabelText( "terrain_pipe", "" );

      setLabelText( "terrain_defencebonus", "" );
      setLabelText( "terrain_attackbonus", "" );
      setLabelText( "terrain_jam", "" );
      setLabelText( "terrain_name", "" );
      setBarGraphColor( "winddisplay", 0x00ff00  );
   }

   if ( mc.valid() && fld ) {
      if ( veh && fieldvisiblenow( fld, actmap->getPlayerView() ) ) {
         showUnitData( veh, NULL, fld );
      } else {

         Building* bld = fld->building;
         if ( bld && fieldvisiblenow( fld, actmap->getPlayerView() ) ) 
            showUnitData( NULL, bld, fld );
         else
            showUnitData( NULL, NULL, fld );
      }
   }

   // PG_Application::SetBulkMode(false);
   // Redraw(true);
}
void DashboardPanel::showUnitData( Vehicle* veh, Building* bld, MapField* fld,  bool redraw )
{
   int weaponsDisplayed = 0;
   this->veh = veh;
   this->bld = bld;
   
   bool bulk = PG_Application::GetBulkMode();
   if ( redraw )
      PG_Application::SetBulkMode(true);
      

   if ( veh ) {
      setLabelText( "unittypename", veh->typ->name );
      
      if ( !veh->privateName.empty() && veh->getOwner() == veh->getMap()->getPlayerView() ) 
         setLabelText( "unitname", ">" + veh->privateName + "<" );
      else if ( !veh->name.empty() )
         setLabelText( "unitname", veh->name );
      else
         setLabelText( "unitname", veh->typ->description );
      
      
      setBargraphValue( "unitdamage", float(100-veh->damage) / 100  );
      setLabelText( "unitstatus", 100-veh->damage );

      setBargraphValue( "unitfuel", veh->getStorageCapacity().fuel ? float( veh->getTank().fuel) / veh->getStorageCapacity().fuel : 0  );
      setLabelText( "unitfuelstatus", veh->getTank().fuel );
      setBargraphValue( "unitmaterial", veh->getStorageCapacity().material ? float( veh->getTank().material) / veh->getStorageCapacity().material : 0  );
      setLabelText( "unitmaterialstatus", veh->getTank().material );
      setBargraphValue( "unitenergy", veh->getStorageCapacity().energy ? float( veh->getTank().energy) / veh->getStorageCapacity().energy : 0  );
      setLabelText( "unitenergystatus", veh->getTank().energy );

      int endurance = UnitHooveringLogic::getEndurance( veh );
      if ( endurance >= 0 )
         setLabelText( "unitEndurance", endurance );
      else
         setLabelText( "unitEndurance", "-" );


      ASCString moveString = ASCString::toString( veh->getMovement() / 10 );
      if ( veh->getMovement() % 10 )
         moveString += "." + ASCString::toString(veh->getMovement()%10);

      setLabelText( "movepoints", moveString );

      if ( veh->typ->fuelConsumption )
         setLabelText( "fuelrange", veh->getTank().fuel / veh->typ->fuelConsumption );
      else
         setLabelText( "fuelrange", "-" );

      setLabelText( "armor", veh->typ->armor );

      int &pos = weaponsDisplayed;
      for ( int i = 0; i < veh->typ->weapons.count; ++i) {
         if ( !veh->typ->weapons.weapon[i].service() && pos < 10 ) {
            ASCString ps = ASCString::toString(pos);
            setLabelText( "punch_weapon" + ps, veh->typ->weapons.weapon[i].maxstrength );
            if ( veh->typ->hasFunction( ContainerBaseType::NoReactionfire  ) || !veh->typ->weapons.weapon[i].shootable() || veh->typ->weapons.weapon[i].getScalarWeaponType() == cwminen )
               setLabelText( "RF_weapon" + ps, "-" );
            else
               setLabelText( "RF_weapon" + ps, veh->typ->weapons.weapon[i].reactionFireShots );
            setLabelText( "status_ammo" + ps, veh->ammo[i] );
            setBargraphValue( "bar_ammo" + ps, veh->typ->weapons.weapon[i].count ? float(veh->ammo[i]) / veh->typ->weapons.weapon[i].count : 0 );
            ++pos;
         }
      }
      
      if ( veh->typ->infoImageSmallFilename.length() > 0 ) {
         setImage( "Selected3DImageSmall", veh->typ->infoImageSmallFilename, this );
         show( "Selected3DImageSmall" );
      } else
         hide( "Selected3DImageSmall" );
      
   } else {
      if ( bld ) {
         setLabelText( "unittypename", bld->typ->name );
         
         if ( !bld->privateName.empty() && bld->getOwner() == bld->getMap()->getPlayerView()) 
            setLabelText( "unitname", ">" + bld->privateName + "<" );
         else 
            setLabelText( "unitname", bld->name );
         
         setBargraphValue( "unitdamage", float(100-bld->damage) / 100  );
         setLabelText( "unitstatus", 100-bld->damage );
         setLabelText( "armor", bld->getArmor() );

         if ( 0 ) {
            setLabelText( "unitfuelstatus", bld->getAvailableResource(maxint,2) );
            setLabelText( "unitmaterialstatus", bld->getAvailableResource(maxint,1) );
            setLabelText( "unitenergystatus", bld->getAvailableResource(maxint,0) );
         } else {
            setLabelText( "unitfuelstatus", "" );
            setLabelText( "unitmaterialstatus", "" );
            setLabelText( "unitenergystatus", "" );
         }


      } else {
         setLabelText( "armor", "" );
         setLabelText( "unittypename", "" );
         setLabelText( "unitname", "" );
         bool objectFound = false;
         if ( fld && fld->objects.size() && fieldvisiblenow( fld )) {
            for ( MapField::ObjectContainer::iterator i = fld->objects.begin(); i != fld->objects.end(); ++i )
               if ( i->typ->armor > 0 ) {
                  setBargraphValue( "unitdamage", float(100-i->damage) / 100  );
                  objectFound = true;
                  break;
               }
         } 
         if ( !objectFound ) 
            setBargraphValue( "unitdamage", 0  );
         setLabelText( "unitstatus", "" );

         setLabelText( "unitfuelstatus", "" );
         setLabelText( "unitmaterialstatus", "" );
         setLabelText( "unitenergystatus", "" );

      }

      setBargraphValue( "unitfuel", 0  );
      setBargraphValue( "unitmaterial",  0  );
      setBargraphValue( "unitenergy",  0  );
      setLabelText( "unitEndurance", "" );

      setLabelText( "fuelrange", "-" );
      setLabelText( "movepoints", "" );
      hide( "Selected3DImageSmall" );
   }
   for ( int i = weaponsDisplayed; i < 10; ++i ) {
      ASCString ps = ASCString::toString(i);
      setLabelText( "punch_weapon" + ps, "" );
      setLabelText( "RF_weapon" + ps, "" );
      setLabelText( "status_ammo" + ps, "" );
      setBargraphValue( "bar_ammo" + ps, 0 );
   }
   
   if ( redraw ) {
      if ( !bulk )
         PG_Application::SetBulkMode(false);
      Redraw(true);
   }
}



WindInfoPanel::WindInfoPanel (PG_Widget *parent, const PG_Rect &r ) : DashboardPanel( parent, r, "WindInfo" )
{
}




UnitInfoPanel::UnitInfoPanel (PG_Widget *parent, const PG_Rect &r ) : DashboardPanel( parent, r, "UnitInfo" )
{
   SpecialInputWidget* siw = dynamic_cast<SpecialInputWidget*>( FindChild( "weapinfo", true ) );
   if ( siw ) {
      siw->sigMouseButtonDown.connect( sigc::mem_fun( *this, &UnitInfoPanel::onClick ));
      siw->sigMouseButtonUp.connect( sigc::mem_fun( *this, &UnitInfoPanel::onClick ));
   }

   PG_Label* l = dynamic_cast<PG_Label*>( parent->FindChild( "unitname", true ) );
   if ( l ) {
      l->sigMouseButtonUp.connect( sigc::hide( sigc::hide( sigc::mem_fun( *this, &UnitInfoPanel::unitNaming ))));
   }

   VehicleTypeSelectionItemFactory::showVehicleInfo.connect( sigc::mem_fun( *this, &UnitInfoPanel::showUnitInfo ));
}

void UnitInfoPanel::showUnitInfo( const VehicleType* vt )
{
   // showUnitData( vt, NULL, true );
}

bool UnitInfoPanel::unitNaming()
{
   ContainerBase* container = NULL;
   if ( veh )
      container = veh;
   else
      if ( bld )
         container = bld;
   
   if ( container && RenameContainerCommand::avail( container ) && container->getOwner() == container->getMap()->actplayer ) {
      UnitNaming un( container );
      un.Show();
      un.RunModal();
      showUnitData( veh, bld, NULL, true );
   }
   return true;
}


bool UnitInfoPanel::onClick ( PG_MessageObject* obj, const SDL_MouseButtonEvent* event )
{

   static const bool modalWeaponInfo = true;
   
   SpecialInputWidget* siw = dynamic_cast<SpecialInputWidget*>(obj);
   if ( siw ) {
      if ( event->button == SDL_BUTTON_RIGHT ) {
         if ( event->type == SDL_MOUSEBUTTONDOWN  ) {

            MapField* fld = actmap->getField( actmap->player[actmap->actplayer].cursorPos );
            const Vehicle* vehicle = veh;
            const VehicleType* vt = veh ? veh->typ : NULL;
            if ( !veh && fld && fld->vehicle ) {
               vt = fld->vehicle->typ;
               vehicle = fld->vehicle;
            }
            if ( vt || veh ) {
               // parent? PG_Application::GetWidgetById( ASC_PG_App::mainScreenID )
               WeaponInfoPanel* wip = new WeaponInfoPanel( NULL, vehicle, vt );
               wip->Show();
               wip->BringToFront();
               if ( modalWeaponInfo ) {
                  wip->SetCapture();
                  wip->RunModal();
                  delete wip;
               } // else
              //     PG_Application::
            }
            return true;
         }
         if ( event->type == SDL_MOUSEBUTTONUP && !modalWeaponInfo ) {
            bool result = false;
            PG_Widget* wip;
            do  {
               wip = PG_Application::GetWidgetByName( WeaponInfoPanel::WIP_Name() );
               if ( wip ) {
                  delete wip;
                  result = true;
               }
            } while ( wip );
            return result;
         }
      }
   }
   return false;
}

class WeaponInfoLine: public PG_Image {
      const SingleWeapon* weapon;
      const VehicleType* veh;
      WeaponInfoPanel* wip;
      static WeaponInfoLine* displayed;
   public:
      WeaponInfoLine( WeaponInfoPanel* parent, const PG_Point& p, SDL_Surface* image, const SingleWeapon* weap, const VehicleType* vehicle )
           : PG_Image( parent, p, image, false ), weapon(weap), veh ( vehicle ), wip(parent)
      {
      };

      void painter ( const PG_Rect &src, const ASCString& name, const PG_Rect &dst)
      {
         Surface screen = Surface::Wrap( PG_Application::GetScreen() );
         if ( name == "weapon_symbol1" )
            screen.Blit( IconRepository::getIcon(SingleWeapon::getIconFileName( weapon->getScalarWeaponType()) + "-small.png"), SPoint(dst.x, dst.y));

         if ( name == "weapon_targets" || name == "weapon_shootfrom" ) {
            int height;
            if (name == "weapon_targets")
               height = weapon->targ;
            else {
               height = weapon->sourceheight;
               if ( veh )
                  height &= veh->height;
            }

            for ( int i = 0; i < 8; ++i )
               if ( height & (1 << i )) {
                  Surface& tick = IconRepository::getIcon("weapon_ok.png");
                  screen.Blit( tick, SPoint(dst.x + i * tick.w(), dst.y  ) );
               }
         }

      };

      void registerSpecialDisplay( const ASCString& name )
      {
         SpecialDisplayWidget* sdw = dynamic_cast<SpecialDisplayWidget*>( FindChild( name, true ) );
         if ( sdw )
            sdw->display.connect( sigc::mem_fun( *this, &WeaponInfoLine::painter ));
      };

	   void eventMouseEnter()
      {
         wip->showWeapon( weapon );
         displayed = this;
      };

      void eventMouseLeave()
      {
         if ( displayed == this )
            wip->showWeapon();
      };

      bool activate()
      {
         if ( displayed != this ) {
            wip->showWeapon( weapon);
            displayed = this;
            return true;
         } else
            return false;
      }
};

WeaponInfoLine* WeaponInfoLine::displayed = NULL;



WeaponInfoPanel::WeaponInfoPanel (PG_Widget *parent, const Vehicle* veh, const VehicleType* vt ) : Panel( parent, PG_Rect::null, "WeaponInfo" ), weaponCount(0)
{
   SetName(name);

   vector<const SingleWeapon*> displayedWeapons;
   vector<int> displayedWeaponNum;

   for ( int j = 0; j < vt->weapons.count ; j++)
      if ( vt->weapons.weapon[j].getScalarWeaponType() >= 0 ) {
         ++weaponCount;
         displayedWeapons.push_back( &vt->weapons.weapon[j] );
         displayedWeaponNum.push_back(j);
      }


   Surface& head = IconRepository::getIcon("weapon_large_top.png");
   Surface& line = IconRepository::getIcon("weapon_large_line.png");
   Surface& foot = IconRepository::getIcon("weapon_large_bottom.png");
   int height = head.h() + foot.h() + weaponCount * line.h() + GetTitlebarHeight();

   SizeWidget( head.w(), height, false );

   int lineStartY = GetTitlebarHeight() + head.h()  - 1;

   PG_Widget* footWidget = FindChild( "bottom", true );
   assert( footWidget != NULL );
   footWidget->MoveWidget(0,  lineStartY + line.h() * weaponCount, false );

   for ( int i = 0; i < weaponCount; ++i ) {
      WidgetParameters widgetParams = getDefaultWidgetParams();
      WeaponInfoLine* lineWidget = new WeaponInfoLine ( this, PG_Point( 0,  lineStartY + i * line.h() ), line.getBaseSurface(), displayedWeapons[i], vt );

      PropertyReadingContainer pc ( "panel", textPropertyGroup );

      pc.openBracket("LineWidget");
      parsePanelASCTXT( pc, lineWidget, widgetParams );
      pc.closeBracket();


      assignWeaponInfo( this, lineWidget, *displayedWeapons[i] );
      lineWidget->registerSpecialDisplay( "weapon_shootfrom" );
      lineWidget->registerSpecialDisplay( "weapon_targets" );
      if ( veh )
         setLabelText( "weapon_currentammo", veh->ammo[displayedWeaponNum[i]], lineWidget );

      weaponInfoLines.push_back( lineWidget );
   }
   setLabelText( "weapon_shootaftermove", vt->wait ? "no" : "yes" );
   setLabelText( "weapon_moveaftershoot", vt->hasFunction( ContainerBaseType::MoveAfterAttack  ) ? "yes" : "no" );

   /*
   for ( int i = 0; i < cmovemalitypenum; ++i ) 
      setLabelText( ASCString("weapon_efficiency_") + unitCategoryTags[i], cmovemalitypes[i] );
      */
}


void WeaponInfoPanel::showWeapon( const SingleWeapon* weap )
{
   PG_Application::SetBulkMode(true);
   int effic[13];
   for ( int k = 0; k < 13; k++ )
      if ( weap )
         effic[k] = weap->efficiency[k];
      else
         effic[k] = -1;

   if ( weap ) {
      int mindelta = 1000;
      int maxdelta = -1000;
      for ( int h1 = 0; h1 < 8; h1++ )
         for ( int h2 = 0; h2 < 8; h2++ )
            if ( weap->sourceheight & ( 1 << h1 ) )
               if ( weap->targ & ( 1 << h2 )) {
                  int delta = getheightdelta ( h1, h2);
                  if ( delta > maxdelta )
                     maxdelta = delta;
                  if ( delta < mindelta )
                     mindelta = delta;
               }
      for ( int a = -6; a < mindelta; a++ )
         effic[6+a] = -1;
      for ( int b = maxdelta+1; b < 7; b++ )
         effic[6+b] = -1;
   }

   for ( int i = -6; i <= 6; ++i ) {
      if ( effic[6+i] >= 0 )
         setLabelText( "weapon_distance_" + ASCString::toString(i), i );
      else
         setLabelText( "weapon_distance_" + ASCString::toString(i), "" );

      if ( weap && effic[6+i] >= 0 )
         setLabelText( "weapon_efficiency_" + ASCString::toString(i), weap->efficiency[6+i]  );
      else
         setLabelText( "weapon_efficiency_" + ASCString::toString(i), "" );
   }

                                   // grey light grey  yellow,    blue      red        green
   static const int colors[6] = { 0x969595, 0xdfdfdf, 0xfac914,  0x5383e6,   0xff5e5e, 0x08ce37 };

   for ( int i = 0; i< cmovemalitypenum; ++i)
      if ( weap ) {
         int col;
         if ( weap->targetingAccuracy[i] < 10 )
            col = colors[0] ;
         else
            if ( weap->targetingAccuracy[i] < 30 )
               col = colors[1] ;
            else
               if ( weap->targetingAccuracy[i] < 80 )
                  col = colors[3] ;
               else
                  if ( weap->targetingAccuracy[i] < 120 )
                     col = colors[2];
                  else
                     col = colors[4];
         setLabelColor( ASCString("weapon_efficiency_") + unitCategoryTags[i], col );
         setLabelText( ASCString("weapon_efficiency_") + unitCategoryTags[i], weap->targetingAccuracy[i]  );
      } else
         setLabelText( ASCString("weapon_efficiency_") + unitCategoryTags[i], "" );

   if ( weap )
      setLabelText( "weapon_text2",  weap->getName() );
   else
      setLabelText( "weapon_text2",  "" );

   if ( weap )  {
      setImage( "weapon_symbol2", IconRepository::getIcon(SingleWeapon::getIconFileName( weap->getScalarWeaponType()) + "-small.png") );
      show( "weapon_symbol2" );
   } else
      hide( "weapon_symbol2" );

   PG_Application::SetBulkMode(false);
   Update();
}

bool   WeaponInfoPanel::eventMouseButtonUp (const SDL_MouseButtonEvent *button)
{
   if ( Panel::eventMouseButtonUp( button ))
      return true;
   
   if ( button->button == SDL_BUTTON_RIGHT ) {
      QuitModal();
      return true;
   } else
      return false;
}


bool WeaponInfoPanel::eventMouseMotion(const SDL_MouseMotionEvent* motion)
{
   for ( int i = 0; i < weaponInfoLines.size();++i )
      if ( weaponInfoLines[i]->IsMouseInside() )
         return weaponInfoLines[i]->activate();
   return false;
};


ASCString WeaponInfoPanel::name = "WeaponInfoPanel";
const ASCString& WeaponInfoPanel::WIP_Name()
{
   return name;
}


MapInfoPanel::MapInfoPanel (PG_Widget *parent, const PG_Rect &r, MapDisplayPG* mapDisplay ) : DashboardPanel( parent, r, "MapInfo" ), zoomSlider(NULL), changeActive(false)
{
   assert( mapDisplay );
   this->mapDisplay = mapDisplay;
   
   zoomSlider = dynamic_cast<PG_Slider*>( FindChild( "zoomscroller", true ) );
   if ( zoomSlider ) {
      zoomSlider->SetRange(0,75); // results in zoomlevels from 100 - 25
      zoomSlider->sigSlide.connect( sigc::mem_fun( *this, &MapInfoPanel::scrollTrack ));
      mapDisplay->newZoom.connect( sigc::mem_fun( *this, &MapInfoPanel::zoomChanged ));
      zoomSlider->SetPosition( 100 - mapDisplay->getZoom() );
   }   

   const int labelnum = 5;
   const char* label[labelnum] = { "pipes", "container", "resources", "visibilityvalue", "reactionfire" };
   for ( int i = 0; i < labelnum; ++i ) {
      PG_CheckButton* cb = dynamic_cast<PG_CheckButton*>( FindChild( label[i], true ) );
      if ( mapDisplay->layerActive( label[i] )
         )
         layerChanged( true, label[i]);
      if ( cb ) 
         cb->sigClick.connect( sigc::bind( sigc::mem_fun( *this, &MapInfoPanel::checkBox ), label[i] ));
   }      
   
   mapDisplay->layerChanged.connect( sigc::mem_fun( *this, &MapInfoPanel::layerChanged ));
      
   PG_Button* b = dynamic_cast<PG_Button*>( FindChild( "weaprange", true ) );
   if ( b )
      b->sigClick.connect( sigc::hide( sigc::mem_fun( *this, &MapInfoPanel::showWeaponRange )));
   
   PG_Button* b2 = dynamic_cast<PG_Button*>( FindChild( "moverange", true ) );
   if ( b2 )
      b2->sigClick.connect( sigc::hide( sigc::mem_fun( *this, &MapInfoPanel::showMovementRange )));
   
}

void MapInfoPanel::layerChanged( bool state, const ASCString& label )
{
   PG_CheckButton* cb = dynamic_cast<PG_CheckButton*>( FindChild( label, true ) );
   if ( cb && ! changeActive ) {
      if ( state )
         cb->SetPressed();
      else
         cb->SetUnpressed();
   }
}

bool MapInfoPanel::showWeaponRange()
{
   executeUserAction( ua_viewunitweaponrange );
   return true;
}

bool MapInfoPanel::showMovementRange()
{
   executeUserAction( ua_viewunitmovementrange );
   return true;
}


void MapInfoPanel::zoomChanged( int zoom )
{
   if ( !changeActive )
      if ( zoomSlider )
         zoomSlider->SetPosition( 100 - zoom );
}

bool MapInfoPanel::scrollTrack( long pos )
{
   changeActive = true;
   mapDisplay->setNewZoom( 100 - pos );
   repaintMap();
   changeActive = false;
   return true;
}

bool MapInfoPanel::checkBox( bool state, const char* name )
{
   changeActive = true;
   mapDisplay->activateMapLayer( name, state );
   repaintMap();
   changeActive = false;
   return true;
}


ActionInfoPanel::ActionInfoPanel (PG_Widget *parent, const PG_Rect &r ) : DashboardPanel( parent, r, "ActionInfo" )
{
   ActionContainer::actionListChanged.connect( sigc::mem_fun( *this, &ActionInfoPanel::update ));
}

void ActionInfoPanel::update( GameMap* map )
{
   TextRenderer* text = dynamic_cast<TextRenderer*>( FindChild( "ActionList", true ));
   if ( text && map) {
      /*
      vector<ASCString> list;
      map->actions.getActionDescriptions( list );
      
      ASCString s;
      for ( vector<ASCString>::const_iterator i = list.begin(); i != list.end(); ++i )
         s += *i + "\n";
      */
      text->SetText( "Function removed, please use Action/Manage dialog" );
   }
}

