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
#include "Lights.h"

#include "utility/Texture.h"

namespace RenderSystem{

  // Any render settings should be declared here as 'static'
  static int w_width, w_height;

  static GLuint render_out_FBO;
  static GLuint render_out_color;
  static Program* deferred_export = nullptr;
  static Program* deferred_uber = nullptr;
  static Program* pointLightProg = nullptr;
  static Program* sunLightProg = nullptr;
  static Program* deferred_shadow = nullptr;
  static Program* seafloor_deform = nullptr;
  static Geometry sphereGeom = Geometry("" STRIFY(ASSET_DIR) "/sphere.obj");
  static Geometry quadGeom = Geometry("" STRIFY(ASSET_DIR) "/quad.obj");

  static ShaderLibrary* shaderlib = nullptr;
  
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
    MatrixStack causticView;
    MatrixStack causticOrtho;
    MatrixStack shadowOrtho;
    MatrixStack bias;
  };

  const static int CAUSTIC_COUNT = 32;
  static DepthSet depthSet;
  static double currCaustic = 0;
  static string causticDir;
  static std::shared_ptr<Texture> caustics[CAUSTIC_COUNT];
  static std::queue<const Entity*> renderq;

  static GLuint shadowFramebuffer;
  static GLuint shadowTexture;

  void init(ApplicationState &appstate);

  void render(ApplicationState &appstate, GameState &gstate, double elapsedTime);
  
  void updateLighting(Scene* scene);

  void updatePointLights(Scene* scene);

  void updateSunLights(Scene* scene);


  void lightingPassSetGLState(GLuint framebuffer);

  void lightingPassResetGLState();

  void bindPointLight(PointLight* pointLight);

  void bindSunLight(SunLight* pointLight);

  void bindCamera(Scene* scene, Program* prog);

  void bindBuffers(Buffers &buffers, Program* shader);

  std::vector<PointLight*>* gatherPointLights(Scene* scene);

  std::vector<SunLight*>* gatherSunLights(Scene* scene);

  void drawPointLights(const vector<PointLight*> &pointLights);

  void drawPointLight(PointLight* pointLight);

  void drawSunLights(const vector<SunLight*> &sunLights);

  void drawSunLight(SunLight* pointLight);

  void setMVP(Camera* camera);

  void geometryPass(GameState &gstate);

  void drawEntities(Scene* scene, Program* shader);

  void drawEntity(const Entity* entity, Program* shader, bool buildq);

  void drawGroundPlane(const GameState& gstate, Program* program);

  void onResize(GLFWwindow *window, int width, int height);

  void initDepthUniforms(double causticOrtho, double shadowOrtho);

  void updateDepthUniforms();

  void setShadowMap(GameState &gstate);

  void lightingPass(ApplicationState &appstate, GameState &gstate, double elapsedTime);

  void initCaustics();

  void updateCaustic(double elapsedTime, double speedMod);

  void initShadowMap(int width, int height);

  void updateShadowMap();

};

// File scope helper functions should be declared in RenderSystem.cpp


#endif
