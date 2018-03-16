#pragma once
#ifndef GAMESTATE_H_
#define GAMESTATE_H_

#include "utility/common.h"
#include "utility/Timeline.hpp"
#include "Scene.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define GSTATE GameState

class GameState{
 public:
  GameState(){setDirection();};
  GameState(double (*timeSource)()) : fxAnimTime(timeSource), gameTime(timeSource) {setDirection();}
  
  UINT score = 0;
  const glm::vec3 levelOrigin = glm::vec3(0.0906, -0.31318, 48.0);
  const glm::quat levelOrientation = glm::angleAxis(glm::radians(4.92f), glm::vec3(.793, -.051, .607));
  const glm::vec3 levelScale = glm::vec3(75.0, .05, 48.0);
  glm::vec3 levelDirection;
  double levelProgress = 0.0; // How far the player and level have moved. A translation offset. 
  // ect.. 

  Scene* activeScene = NULL; 

  Timeline<double> fxAnimTime; // Timeline used to parametrize effects and animations
  Timeline<double> gameTime; // Timeline used to time out gameplay events and systems and track playtime

private:
  void setDirection(){
    levelDirection = normalize(glm::vec3(1.0, 0.0, 0.0) * glm::toMat3(levelOrientation));
    levelDirection.y = -levelDirection.y; // I don't know why this has to be flipped but it does
    levelDirection.z = -levelDirection.z;
  }
};

#endif
