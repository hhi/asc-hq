//
// C++ Implementation: weathercast
//
// Description:
//
//
// Author: Kevin Hirschmann <hirsch@dhcppc0>, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "weathercast.h"
#include "guidimension.h"
#include "graphics/blitter.h"
#include "iconrepository.h"
#include "spfst.h"

SDL_Color LRAINOR = {255, 0, 0};

const int Weathercast::xSize = 400;
const int Weathercast::ySize = 400;

const int Weathercast::MAPXSIZE = 200;
const int Weathercast::MAPYSIZE = 200;
Weathercast::Weathercast(const WeatherSystem& ws):  ASC_PG_Dialog(NULL, PG_Rect( 100, 100, xSize, ySize ), "Weathercast", MODAL ), weatherSystem(ws), windSpeed(0), counter(0) {

    turnLabel = new PG_Label(this, PG_Rect(xSize /2, GuiDimension::getTopOffSet(), 10, GetTextHeight() * 2), "Turn: +0");
    turnLabel->SetSizeByText();
    turnLabelWidth = turnLabel->Width();
    back = new PG_Button(this, PG_Rect(10, GuiDimension::getTopOffSet()*2, 55, 35), "back", 90);
    back->sigClick.connect(SigC::slot( *this, &Weathercast::buttonBack));
    forward = new PG_Button(this, PG_Rect(Width() -(back->Width() + 10) , GuiDimension::getTopOffSet()*2, 55, 35), "forward", 90);
    forward->sigClick.connect(SigC::slot( *this, &Weathercast::buttonForward));

    Surface screen = Surface::Wrap( PG_Application::GetScreen() );
    actmap->overviewMapHolder.clear();
    s = actmap->overviewMapHolder.getOverviewMap();



    generateWeatherMap(actmap->time.turn());
    s.strech(MAPXSIZE, MAPYSIZE);
    weatherMapImage = new PG_Image(this, PG_Point(25, 100), s.GetSurface(), false, PG_Draw::BkMode(PG_Draw::TILE), "g");

    windRoseImage = new PG_Image(this, PG_Point(300, 150), IconRepository::getIcon("windrose.png").GetSurface(), false, PG_Draw::BkMode(PG_Draw::TILE), "g");
    sdw  = new SpecialDisplayWidget(this, PG_Rect(100, 100, 100, 100));
    sdw->display.connect( SigC::slot( *this, &Weathercast::painter ));

    WindData wData;
    wData.speed = weatherSystem.getCurrentWindSpeed();
    wData.direction = weatherSystem.getCurrentWindDirection();
    windStack.push_front(wData);
    windspeedLabel = new PG_Label(this, PG_Rect(280, 170 + windRoseImage->Height() , 10, GetTextHeight() * 2), "Windspeed: ");
    updateWeatherSpeed(actmap->time.turn());

    okButton = new PG_Button(this, PG_Rect((xSize - GuiDimension::getButtonWidth()) / 2, ySize - (GuiDimension::getButtonHeight() + GuiDimension::getTopOffSet()), GuiDimension::getButtonWidth(), GuiDimension::getButtonHeight()), "OK", 90);
    okButton->sigClick.connect(SigC::slot( *this, &Weathercast::closeWindow ));

    sigClose.connect( SigC::slot( *this, &Weathercast::closeWindow ));
}

Weathercast::~Weathercast() {}

void Weathercast::paintWeatherArea(const WeatherArea* wa) {
    MegaBlitter<4,colorDepth,ColorTransform_None, ColorMerger_AlphaOverwrite> blitter;

    static const char* weathernames[] = {"terrain_weather_dry.png",
                                         "terrain_weather_lightrain.png",
                                         "terrain_weather_heavyrain.png",
                                         "terrain_weather_lightsnow.png",
                                         "terrain_weather_heavysnow.png",
                                         "terrain_weather_ice.png"
                                        };

    WindAccu wAccu = warea2WindAccu[wa];
    int vMove = static_cast<int>(wAccu.verticalValue);
    int hMove = static_cast<int>(wAccu.horizontalValue);
    if((wa->getCenterPos().x + hMove >0) && (wa->getCenterPos().y + vMove >0)) {
        SPoint pixCenter = OverviewMapImage::map2surface(MapCoordinate(wa->getCenterPos().x + hMove, wa->getCenterPos().y + vMove));
        Surface wSurface = IconRepository::getIcon(weathernames[wa->getFalloutType()]);
        wSurface.strech(18,10);
        blitter.blit ( wSurface, s, pixCenter );
    }

    /*for(int i = (wa->getCenterPos().x - wa->getWidth()/2); i < wa->getCenterPos().x + wa->getWidth(); i++){
      for(int j = (wa->getCenterPos().y - wa->getHeight()/2); j < wa->getCenterPos().y + wa->getHeight()/2; j++){   
        SPoint pixCenter = OverviewMapImage::map2surface(MapCoordinate(i,j));
        OverviewMapImage::fill(s, pixCenter, LRAINOR);
      }
    }*/
}

void Weathercast::generateWeatherMap(int turn) {
    for(int i =0; i < weatherSystem.getActiveWeatherAreasSize(); i++) {
        const WeatherArea* wa = weatherSystem.getNthActiveWeatherArea(i);
        if((wa->getDuration() > (turn - actmap->time.turn())) && (wa->getCenterField()->isOnMap(actmap))) {
            paintWeatherArea(wa);
        }        

    }
    for(int i =0; i < weatherSystem.getQueuedWeatherAreasSize(); i++) {
        pair<GameTime, WeatherArea*> p = weatherSystem.getNthWeatherArea(i);
        if(p.first.turn() <= turn) {            
            paintWeatherArea(p.second);
        }
    }
}


void Weathercast::painter (const PG_Rect &src, const ASCString& name, const PG_Rect &dst) {
    /*   cout << "painter" << endl;
       if(windStack.front().speed > 0) {
             Surface screen = Surface::Wrap(this->GetWidgetSurface()  );
    {
           Surface s = actmap->getOverviewMap();  
    MegaBlitter< gamemapPixelSize, gamemapPixelSize,ColorTransform_None,ColorMerger_PlainOverwrite,SourcePixelSelector_DirectZoom> blitter;
           blitter.setSize( 100, 100, 200, 200 );
           blitter.blit( s, screen, SPoint(0, 0) );c++ struct
    }
         
           MegaBlitter<4,colorDepth,ColorTransform_None, ColorMerger_AlphaOverwrite, SourcePixelSelector_DirectRotation> blitter;
           blitter.setAngle( directionangle[windStack.front().direction] );
           PG_Point p(windRoseImage->x, windRoseImage->y);
           blitter.blit ( IconRepository::getIcon("wind-arrow.png"), screen, SPoint(p.x, p.y) );
       }*/
}

WindAccu Weathercast::updateWindAccu(const WindAccu& ac, unsigned int windspeed, Direction windDirection, float ratio) {
    WindAccu wAccu = {ac.horizontalValue, ac.verticalValue};
    if(windDirection == N) {
        wAccu.verticalValue += -1 * static_cast<int>(ratio * windspeed) *2;
    } else if(windDirection == NE) {
        wAccu.verticalValue += -1 * static_cast<int>(ratio * windspeed) *2;
        wAccu.horizontalValue += static_cast<int>(ratio * windspeed);
    } else if(windDirection == SE) {
        wAccu.verticalValue +=  static_cast<int>(ratio * windspeed) *2;
        wAccu.horizontalValue += static_cast<int>(ratio * windspeed);
    } else if(windDirection == S) {
        wAccu.verticalValue += static_cast<int>(ratio * windspeed) *2;
    } else if(windDirection == SW) {
        wAccu.verticalValue += static_cast<int>(ratio * windspeed) *2;
        wAccu.horizontalValue += -1 * static_cast<int>(ratio * windspeed);
    } else if(windDirection == NW) {
        wAccu.verticalValue += -1 * static_cast<int>(ratio * windspeed) *2;
        wAccu.horizontalValue += -1 * static_cast<int>(ratio * windspeed);
    }
    return wAccu;
}

bool Weathercast::buttonForward( PG_Button* button ) {
    if(counter < weatherSystem.getMaxForecast()) {
       //update turn information
        ++counter;
        showTurn();
	//update wind data
        updateWeatherSpeed(actmap->time.turn() + counter);
	//update display data of active weatherareas (active at least since actmap->time)
        for(int i =0; i < weatherSystem.getActiveWeatherAreasSize(); i++) {
            const WeatherArea* wa = weatherSystem.getNthActiveWeatherArea(i);
            if(wa->getDuration() - counter > 0) {
                WindAccu wAccu;
                if(counter == 1) {
                    wAccu.horizontalValue = wa->getHorizontalWindAccu();
                    wAccu.verticalValue = wa->getVerticalWindAccu();
                } else {
                    wAccu = warea2WindAccu[wa];
                }
                wAccu = updateWindAccu(wAccu, windSpeed,  windStack.front().direction, weatherSystem.getWindspeed2FieldRatio());
                warea2WindAccu[wa] = wAccu;
            }
        }
	//update display data for queued weatherareas
        for(int i =0; i < weatherSystem.getQueuedWeatherAreasSize(); i++) {
            pair<GameTime, WeatherArea*> p = weatherSystem.getNthWeatherArea(i);
            const WeatherArea* wa = p.second;
            WindAccu wAccu;
            if(p.first.turn() == actmap->time.turn() + counter) {
                wAccu.horizontalValue = 0;
                wAccu.verticalValue = 0;
            } else {
                wAccu.horizontalValue = warea2WindAccu[p.second].horizontalValue;
                wAccu.verticalValue = warea2WindAccu[p.second].verticalValue;
            }
            wAccu = updateWindAccu(wAccu, windSpeed,  windStack.front().direction, weatherSystem.getWindspeed2FieldRatio());
            warea2WindAccu[p.second] = wAccu;
        }
        //display new data
        actmap->overviewMapHolder.clear();
        s = actmap->overviewMapHolder.getOverviewMap();
        generateWeatherMap(actmap->time.turn() + counter);
        s.strech(MAPXSIZE, MAPYSIZE);
        weatherMapImage->SetImage(s.getBaseSurface());
        Update();
    }
    return true;
}

bool Weathercast::buttonBack( PG_Button* button ) {
    if(counter>0) {
        //update turn information
        --counter;
        showTurn();
        //update display data of active weatherareas (active at least since actmap->time)
        for(int i =0; i < weatherSystem.getActiveWeatherAreasSize(); i++) {
            const WeatherArea* wa = weatherSystem.getNthActiveWeatherArea(i);
            if(wa->getDuration() - counter > 0) {
                WindAccu wAccu = warea2WindAccu[wa];
                wAccu = updateWindAccu(wAccu, windSpeed,  static_cast<Direction>((windStack.front().direction +3) % 6), weatherSystem.getWindspeed2FieldRatio());
                warea2WindAccu[wa] = wAccu;
            }
        }
        //update display data for queued weatherareas
        for(int i =0; i < weatherSystem.getQueuedWeatherAreasSize(); i++) {
            pair<GameTime, WeatherArea*> p = weatherSystem.getNthWeatherArea(i);
            const WeatherArea* wa = p.second;
            WindAccu wAccu;
            if(p.first.turn() <= actmap->time.turn() + counter) {
                WindAccu wAccu = warea2WindAccu[wa];
                wAccu = updateWindAccu(wAccu, windSpeed,  static_cast<Direction>((windStack.front().direction +3) % 6), weatherSystem.getWindspeed2FieldRatio());
                warea2WindAccu[wa] = wAccu;
            }
        }
        //display new data
        actmap->overviewMapHolder.clear();
        s = actmap->overviewMapHolder.getOverviewMap();
        generateWeatherMap(actmap->time.turn() + counter);
        s.strech(MAPXSIZE, MAPYSIZE);
        weatherMapImage->SetImage(s.getBaseSurface());
        //update wind data
        if(weatherSystem.getWindDataOfTurn(actmap->time.turn() + counter + 1).speed >=0) {
            windStack.pop_front();
            windSpeed = windStack.front().speed;
            showWindSpeed();
        }
        Update();
    }
    return true;
}

bool Weathercast::closeWindow() {
    quitModalLoop(1);
    return true;

}

void Weathercast::showTurn() {
    std::string turn = "Turn: +";
    turn.append(strrr(counter));
    turnLabel->SetText(turn);
    turnLabel->SetSizeByText();
    turnLabel->Redraw();

}

void Weathercast::showWindSpeed() {
    std::string wind = "WindSpeed: ";
    wind.append(strrr(windSpeed));
    windspeedLabel->SetText(wind);
    windspeedLabel->SetSizeByText();
    windspeedLabel->Redraw();

}


void Weathercast::updateWeatherSpeed(int turn) {
    int newWindSpeed = weatherSystem.getWindDataOfTurn(turn).speed;
    WindData wd = weatherSystem.getWindDataOfTurn(turn);
    windSpeed = newWindSpeed;
    windStack.push_front(wd);
    windRoseImage->Update();    
    showWindSpeed();
}


extern void weathercast() {
    Weathercast wc(*(actmap->weatherSystem));
    wc.Show();
    wc.Run();
}


