/***************************************************************************
                          CLoadable.cpp  -  description
                             -------------------
    begin                : Thu Jun 29 2000
    copyright            : (C) 2000 by frank landgraf
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <stdio.h>
#include "CLoadable.h"

#include "Property.h"
#include "gameoptions.h"

CLoadableGameOptions::CLoadableGameOptions(CGameOptions* pOptions)
	:	_pOptions(pOptions)
{
	add(new IntProperty("FastMove"								,&_pOptions->fastmove,3));
	add(new IntProperty("VisibilityCalculationAlgorithm"	,&_pOptions->visibility_calc_algo));
	add(new IntProperty("MovementSpeed"							,&_pOptions->movespeed));
	add(new IntProperty("EndTurnPrompt"							,&_pOptions->endturnquestion));
	add(new IntProperty("SmallMapVisible"						,&_pOptions->smallmapactive));
	add(new IntProperty("UnitsGrayAfterMove"					,&_pOptions->units_gray_after_move));
	add(new IntProperty("MapZoom"									,&_pOptions->mapzoom));
	add(new IntProperty("MapZoomEditor"							,&_pOptions->mapzoomeditor));
	add(new IntProperty("StartupCount"							,&_pOptions->startupcount));
	add(new IntProperty("dontMarkFieldsNotAccessible_movement"	,&_pOptions->dontMarkFieldsNotAccessible_movement));
	add(new IntProperty("AttackSpeed1"							,&_pOptions->attackspeed1));
	add(new IntProperty("AttackSpeed2"							,&_pOptions->attackspeed2));
	add(new IntProperty("AttackSpeed3"							,&_pOptions->attackspeed3));
	
	add(new IntProperty("Mouse.ScrollButton"					,&_pOptions->mouse.scrollbutton));
	add(new IntProperty("Mouse.SelectFieldButton"			,&_pOptions->mouse.fieldmarkbutton));
	add(new IntProperty("Mouse.SmallGuiIconButton"			,&_pOptions->mouse.smallguibutton));
	add(new IntProperty("Mouse.LargeGuiIconButton"			,&_pOptions->mouse.largeguibutton));
	add(new IntProperty("Mouse.SmallGuiIconUnderMouse"		,&_pOptions->mouse.smalliconundermouse ));
	add(new IntProperty("Mouse.MapCenterButton"				,&_pOptions->mouse.centerbutton ));
	add(new IntProperty("Mouse.UnitWeaponInfoButton"		,&_pOptions->mouse.unitweaponinfo));
	// add(new IntProperty("Mouse.dragndropmovement"		,&_pOptions->mouse.dragndropmovement));
	
	add(new IntProperty("Container.ProduceAmmoAutomatically"	,	&_pOptions->container.autoproduceammunition));
	add(new IntProperty("Container.FillUnitsAutomatically"	,	&_pOptions->container.filleverything));
	add(new IntProperty("Container.EmptyUnitsAutomatically"	,	&_pOptions->container.emptyeverything));
	
	add(new IntProperty("ToolTipHelpDelay"					,	&_pOptions->onlinehelptime));
	add(new IntProperty("SmallGuiIconOpensAfterMove"	,	&_pOptions->smallguiiconopenaftermove));
	add(new IntProperty("DefaultPassword"					,	&_pOptions->defaultpassword));
	add(new IntProperty("ReplayDelay"						,	&_pOptions->replayspeed));
   add(new IntProperty("ShowUnitOwner"                ,  &_pOptions->showUnitOwner));
	
	add(new TextProperty("BI3.path"							,	&_pOptions->bi3.dir	)	);
	add(new IntProperty("BI3.interpolate.terrain"		,	&_pOptions->bi3.interpolate.terrain));
	add(new IntProperty("BI3.interpolate.units"			,	&_pOptions->bi3.interpolate.units));
	add(new IntProperty("BI3.interpolate.objects"		,	&_pOptions->bi3.interpolate.objects));
	add(new IntProperty("BI3.interpolate.buildings"		,	&_pOptions->bi3.interpolate.buildings));
	
	char buf[1000];
	for ( int i = 0; i < _pOptions->getSearchPathNum(); i++ ) {
		sprintf(buf,"SearchPath%d", i );
		add(new TextProperty(buf						,	&_pOptions->searchPath[i]	)	);
   }
}

CLoadableGameOptions::~CLoadableGameOptions()
{}
	  
