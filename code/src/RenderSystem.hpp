#pragma once 
#ifndef RENDERSYSTEM_H_
#define RENDERSYSTEM_H_

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

#include "utility/Texture.h"

namespace RenderSystem{

  // Any render settings should be declared here as 'static'
  static int w_width, w_height;

  static GLuint render_out_FBO;
  static GLuint render_out_color;
  static Program* deferred_export = NULL;
  static Program* deferred_uber = NULL;
  static Program* deferred_shadow = NULL;

  static ShaderLibrary* shaderlib = NULL;
  
  struct Buffers {
    std::vector<unsigned int> buffers;
    unsigned int gBuffer, depthBuffer;
  };

  static Buffers deferred_buffers;

  struct MVPset {
    MatrixStack M;
    MatrixStack V;
    MatrixStack P;
  };

  static MVPset MVP;

  struct DepthSet {
    MatrixStack lightView;
    MatrixStack causticOrtho;
    MatrixStack shadowOrtho;
    MatrixStack bias;
  };

  const static int CAUSTIC_COUNT = 32;
  static DepthSet depthSet;
  static int currCaustic = 0;
  static string causticDir;
  static std::shared_ptr<Texture> caustics[CAUSTIC_COUNT];
  static std::queue<const Entity*> renderq;

  static GLuint shadowFramebuffer;
  static GLuint shadowTexture;

  void init(ApplicationState &appstate);

  void render(ApplicationState &appstate, GameState &gstate, double elapsedTime);

  void updateLighting(Scene* scene);

  void setMVP(Camera* camera);

  void geometryPass(GameState &gstate);

  void applyShading(Scene* scene, ShaderLibrary &shaderlib);

  void drawEntities(Scene* scene, Program* shader);

  void drawEntity(const Entity* entity, Program* shader, bool buildq);

  void onResize(GLFWwindow *window, int width, int height);

  void initDepthUniforms(double causticOrtho, double shadowOrtho);

  void updateDepthUniforms();

  void initCaustics();

  void updateCaustic();

  void initShadowMap(int width, int height);

  void updateShadowMap();

};

// File scope helper functions should be declared in RenderSystem.cpp


#endif
