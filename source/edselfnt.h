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

#ifndef edselfntH
#define edselfntH

#include <algorithm>

#include <paragui.h>
#include <pgwidget.h>
#include <pgwidgetlist.h>
#include <pgwindow.h>
#include <pgapplication.h>

#include "dialogs/selectionwindow.h"
#include "ed_mapcomponent.h"
#include "edglobal.h"

#include "events.h"
#include "itemrepository.h"
#include "objects.h"

#define selfontyanf 40       						//Auswahlanzeige Yanf
#define selfontxsize 280 						//Breite des Auswahlbalkens am Rand
#define selfontxanf ( agmp->resolutionx - selfontxsize - 10 )  	//Auswahlanzeige Xanf

extern void selcargo( PG_Window* parentWindow, ContainerBase* container );
extern void selbuildingproduction( Building* eht );

extern void sortItems( vector<Vehicletype*>& vec );
extern void sortItems( vector<BuildingType*>& vec );
extern void sortItems( vector<ObjectType*>& vec );
extern void sortItems( vector<TerrainType*>& vec );
extern void sortItems( vector<MineType*>& vec );






template <class MapItemType> 
class MapItemTypeWidget : public SelectionWidget {
      const MapItemType* item;
   public:
      typedef MapItemType ItemType;
      
      MapItemTypeWidget ( PG_Widget* parent, const PG_Point& pos, const MapItemType* mapItemType ) : SelectionWidget( parent, PG_Rect( pos.x, pos.y, ItemTypeSelector<ItemType>::type::Width(), ItemTypeSelector<ItemType>::type::Height() + MapComponent::fontHeight )), item( mapItemType )
      {
      
      };
      
      ASCString getName() const 
      {
         return item->getName();
      }
      
      const ItemType* getItem() const 
      {
         return item;
      };      
      
      void display( SDL_Surface * surface, const PG_Rect & src, const PG_Rect & dst ) {
         typename ItemTypeSelector<MapItemType>::type mapComponent( item );
         mapComponent.displayClip( this, surface, src, dst );
      };
      
};



template <class MapItemWidget> 
class BaseMapItemTypeWidgetFactory : public SelectionItemFactory {
   protected:
      typedef typename MapItemWidget::ItemType ItemType;
      typedef vector<ItemType*> Items;
      typename Items::iterator it;

      
      
      virtual bool isFiltered( const ItemType& item ) {
         return ItemFiltrationSystem::isFiltered( &item );
      };

      virtual void registerItem( MapItemWidget* item ) {};
      
   private:      
      const ItemRepository<ItemType>& repository;
      Items items;
      
   public:
      BaseMapItemTypeWidgetFactory( const ItemRepository<ItemType>& itemRepository  )  : repository( itemRepository ) {
         for ( size_t i = 0; i < repository.getNum(); ++i ) {
            ItemType* item = repository.getObject_byPos(i);
            if ( item ) 
               items.push_back(item);
         }
         sortItems( items );
         restart();   
      };

      void restart()
      {
         it = items.begin();
      };
      
      SelectionWidget* spawnNextItem( PG_Widget* parent, const PG_Point& pos )
      {
         while ( it != items.end() && isFiltered( **it) )
            ++it;
            
         if ( it != items.end() ) {
            MapItemWidget* myItem = new MapItemWidget( parent, pos, *(it++) );
            registerItem( myItem );
            return myItem;
         } else
            return NULL;
      };
};

template <class MapItemWidget> 
class MapItemTypeWidgetFactory : public BaseMapItemTypeWidgetFactory<MapItemWidget>  {
   protected:
      typedef typename MapItemWidget::ItemType ItemType;
   public:

      MapItemTypeWidgetFactory( const ItemRepository<ItemType>& itemRepository  )  : BaseMapItemTypeWidgetFactory<MapItemWidget>( itemRepository ) { };

      void itemSelected( const SelectionWidget* widget, bool mouse )
      {
         if ( !widget )
            return;
            
         const MapItemWidget* mapItemWidget = dynamic_cast<const MapItemWidget*>(widget);
         assert( mapItemWidget );
         if ( mapItemWidget->getItem() ) {
            typename ItemTypeSelector<typename MapItemWidget::ItemType>::type item ( mapItemWidget->getItem() );
            selection.setSelection( item );
         }
      }
};


template <class MapItemWidget> 
class MapItemTypeWidgetFactory_IDSelection : public BaseMapItemTypeWidgetFactory<MapItemWidget>  {
      typedef typename MapItemWidget::ItemType ItemType;
      int& itemID;
      int defaultItemID;
      std::map<int,SelectionWidget*> cache;
   protected:
      virtual void registerItem( MapItemWidget* itemWidget )
      {
         cache[itemWidget->getItem()->getID()] = itemWidget;
      };
      
   public:

      MapItemTypeWidgetFactory_IDSelection( const ItemRepository<ItemType>& itemRepository, int& id  )  : BaseMapItemTypeWidgetFactory<MapItemWidget>( itemRepository ), itemID( id ), defaultItemID(id)
      {
      };

      SelectionWidget* getDefaultItem()
      {
         if ( cache.find( defaultItemID ) != cache.end() )
            return cache[defaultItemID];
         else
            return NULL;
      }

      
      void itemSelected( const SelectionWidget* widget, bool mouse )
      {
         if ( !widget )
            return;

         const MapItemWidget* mapItemWidget = dynamic_cast<const MapItemWidget*>(widget);
         assert( mapItemWidget );
         if ( mapItemWidget->getItem() ) 
            itemID = mapItemWidget->getItem()->id;
      }
};

template <class ItemType>
bool selectItemID( int& id, const ItemRepository<ItemType>& itemRepository )
{
   ItemSelectorWindow isw( NULL, PG_Rect( 300, 50, 280, PG_Application::GetScreenHeight()-100), "select item", new MapItemTypeWidgetFactory_IDSelection< MapItemTypeWidget<ItemType> >(itemRepository, id) );
   isw.Show();
   isw.RunModal();
   return true;
}



extern void addCargo( ContainerBase* container );

extern void editProduction( ContainerBase* container );



#endif
