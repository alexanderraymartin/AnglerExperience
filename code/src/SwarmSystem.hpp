#pragma once 
#ifndef _SWARMSYSTEM_H_
#define _SWARMSYSTEM_H_

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

namespace SwarmSystem{

	void update(ApplicationState &appstate, GameState &gstate, double elapsedTime, float ***fishLoc, float ***headLoc, float groundH, float mx, float my);

	void init(ApplicationState &appstate, GameState &gstate, float **fishLoc);

};

#endif