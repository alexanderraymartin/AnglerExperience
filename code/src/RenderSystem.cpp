#include "RenderSystem.hpp"
#include <common.h>
#include <GLSL.h>
#include <Program.h>
#include "components/Geometry.hpp"
#include "core.h"
#include "GameState.hpp"
#include "Entity.hpp"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Forward Declarations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// draw Functions start general and get more specific

static void computeLighting(/*...*/);
static void drawBackground(/*...*/);
static void drawAngler(/*...*/);
static void drawEntities(/*...*/);
static void drawEffects(/*...*/);
static void postProcess(/*...*/);

static void shadowMapping(Scene* scene /*...*/);
static void computeLighting(Scene* scene /*...*/);
static void renderCaustics(Scene* scene /*...*/);

static void drawEntity(const Entity* entity);
static void drawFlock(const Entity* entity);

static void drawGeometry(const Geometry &geomcomp);
static void drawAntennae(/*...*/);
static void drawLamp(/*...*/);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // RenderSystem functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void RenderSystem::init(ApplicationState &appstate){
  glfwGetFramebufferSize(appstate.window, &w_width, &w_height);
  glViewport(0, 0, w_width, w_height);
}

void RenderSystem::render(ApplicationState &appstate, GameState &gstate, double elapsedTime){

  computeLighting();
  drawBackground();
  drawAngler();
  drawEntities();
  drawEffects();
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


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Static Functions and Helpers
// It may be possible to move some parts of this into one or more libraries. 
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static void drawGeometry(const Geometry &geomcomp, Program* shader){
  int h_pos, h_nor, h_tex;
  h_pos = h_nor = h_tex = -1;

   glBindVertexArray(geomcomp.vaoID);
  // Bind position buffer
  h_pos = shader->getAttribute("vertPos");
  GLSL::enableVertexAttribArray(h_pos);
  glBindBuffer(GL_ARRAY_BUFFER, geomcomp.posBufID);
  glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  
  // Bind normal buffer
  h_nor = shader->getAttribute("vertNor");
  if(h_nor != -1 && geomcomp.norBufID != 0) {
    GLSL::enableVertexAttribArray(h_nor);
    glBindBuffer(GL_ARRAY_BUFFER, geomcomp.norBufID);
    glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  }

  if (geomcomp.texBufID != 0) {  
    // Bind texcoords buffer
    h_tex = shader->getAttribute("vertTex");
    if(h_tex != -1 && geomcomp.texBufID != 0) {
      GLSL::enableVertexAttribArray(h_tex);
      glBindBuffer(GL_ARRAY_BUFFER, geomcomp.texBufID);
      glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    }
  }
  
  // Bind element buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geomcomp.eleBufID);
  
  // Draw
  glDrawElements(GL_TRIANGLES, (int)geomcomp.eleBuf->size(), GL_UNSIGNED_INT, (const void *)0);
  
  // Disable and unbind
  if(h_tex != -1) {
    GLSL::disableVertexAttribArray(h_tex);
  }
  if(h_nor != -1) {
    GLSL::disableVertexAttribArray(h_nor);
  }
  GLSL::disableVertexAttribArray(h_pos);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void computeLighting(/*...*/){
  
}

static void drawEffects(/*...*/){
  
}

static void drawBackground(/*...*/){

}

static void drawAngler(/*...*/){

}

static void drawEntities(/*...*/){

}

static void postProcess(/*...*/){

}
