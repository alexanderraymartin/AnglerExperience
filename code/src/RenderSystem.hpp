#pragma once 
#ifndef RENDERSYSTEM_H_
#define RENDERSYSTEM_H_

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

namespace RenderSystem{

	// Any render settings should be declared here as 'static'
	static int w_width, w_height;

	// Any data structures used inbetween renders should also be stored here as static
	static GLuint quadVAO;
	static GLuint quadVBO;

	static GLuint deferred_fbo;
	static Program* deferred_export = NULL;
	static Program* deferred_uber = NULL;

	static ShaderLibrary* shaderlib = NULL;

	struct Buffers {
		std::vector<unsigned int> buffers;
		unsigned int gBuffer, depthBuffer;
	};

	static Buffers deferred_buffers;

	struct MVPset{
		MatrixStack M;
		MatrixStack V;
		MatrixStack P;
	};

	static MVPset MVP;

	void init(ApplicationState &appstate);

	void render(ApplicationState &appstate, GameState &gstate, double elapsedTime);

	void applyShading(Scene* scene, ShaderLibrary &shaderlib);

	void drawEntities(Scene* scene);

	void drawEntity(const Entity* entity);

	void onResize(GLFWwindow *window, int width, int height);

};

// File scope helper functions should be declared in RenderSystem.cpp


#endif