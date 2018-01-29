#pragma once
#ifndef GAMESTATE_H_
#define GAMESTATE_H_

#include "utility/common.h"
#include "utility/Timeline.hpp"
//#include "Scene.hpp"

class GameState{
 public:
  UINT score = 0;
  double levelProgress = 0.0;
  // ect.. 

  // Scene* ActiveScene = NULL; 

  Timeline<double> fxAnimTime; // Timeline used to parametrize effects and animations
  Timeline<double> gameTime; // Timeline used to time out gameplay events and systems and track playtime

  GameState(){};
  GameState(double (*timeSource)()) : fxAnimTime(timeSource), gameTime(timeSource) {}
};

#endif
