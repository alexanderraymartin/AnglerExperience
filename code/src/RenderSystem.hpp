#pragma once 
#ifndef RENDERSYSTEM_H_
#define RENDERSYSTEM_H_

#include <stdlib.h>
#include <common.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include "core.h"
#include "GameState.hpp"
#include "Scene.hpp"
#include "Entity.hpp"
#include "Component.hpp"

namespace RenderSystem{

	// Any render settings should be declared here as 'static'
	static int w_width, w_height;

	// Any data structures used inbetween renders should also be stored here as static
	static GLuint FBOpair[2]; 

	void init(ApplicationState &appstate);

	void render(ApplicationState &appstate, GameState &gstate, double elapsedTime);

	void onResize(GLFWwindow *window, int width, int height);

};

// File scope helper functions should be declared in RenderSystem.cpp


#endif