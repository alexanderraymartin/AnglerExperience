#pragma once 
#ifndef _GAMEPLAYSYSTEM_H_
#define _GAMEPLAYSYSTEM_H_

#include <stdlib.h>
#include <common.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/glm.hpp>
#include <MatrixStack.h>

#include "core.h"
#include "GameState.hpp"
#include "Scene.hpp"
#include "Entity.hpp"
#include "Component.hpp"

namespace GameplaySystem{
  
	void update(ApplicationState &appstate, GameState &gstate, double elapsedTime, float ***fishLoc, float ***headLoc);


};

#endif