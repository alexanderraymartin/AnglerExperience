#include "RenderSystem.hpp"
#include <common.h>
#include "core.h"
#include "GameState.hpp"

static void drawBackground();
static void drawAngler();
static void drawEntities();
static void postProcess();


void RenderSystem::init(ApplicationState &appstate){
  glfwGetFramebufferSize(appstate.window, &w_width, &w_height);
  glViewport(0, 0, w_width, w_height);
}

void RenderSystem::render(ApplicationState &appstate, GameState &gstate, double elapsedTime){

  drawBackground(appstate.resources);
  drawAngler();
  drawEntities();
  postProcess();

  // Added this so it isn't just black screen. 
  glClearColor(
    0.0,
    fabs(sinf(gstate.fxAnimTime.getTime())),
    fabs(sinf(gstate.fxAnimTime.getTime()+M_PI_2)),
    1.0
  );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderSystem::onResize(GLFWwindow *window, int width, int height){
  w_width = width;
  w_height = height;
  glViewport(0, 0, width, height);
}

static void drawBackground(){

}

static void drawAngler(){

}

static void drawEntities(){

}

static void postProcess(){

}
