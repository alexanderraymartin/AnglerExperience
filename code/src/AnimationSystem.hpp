#pragma once 
#ifndef _ANIMATIONSYSTEM_H_
#define _ANIMATIONSYSTEM_H_

#include <stdlib.h>
#include <common.h>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <MatrixStack.h>

#include "core.h"
#include "GameState.hpp"
#include "Scene.hpp"
#include "Entity.hpp"
#include "Component.hpp"

namespace AnimationSystem{

	void update(ApplicationState &appstate, GameState &gstate, double elapsedTime);

};

#endif