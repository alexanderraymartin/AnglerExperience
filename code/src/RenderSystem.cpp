#include "RenderSystem.hpp"
#include <common.h>
#include <GLSL.h>
#include <Program.h>
#include "core.h"
#include "GameState.hpp"
#include "Entity.hpp"

#include "components/SimpleComponents.hpp"
#include "components/Geometry.hpp"
#include "LightingComponents.hpp"
#include "PostProcessor.h"

// Disable complaints about all the placeholder functions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Forward Declarations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

PostProcessor* postProcessor;

// draw Functions start general and get more specific

static void computeLighting(/*...*/);
static void drawBackground(/*...*/);
static void drawAngler(/*...*/);
static void drawEntities(Scene* scene, RenderSystem::MVPset &MVP, ShaderLibrary &shaderlib);
static void drawEffects(/*...*/);
static void postProcess(/*...*/);

static glm::mat4 updateView(Component* cmpnt);

static void shadowMapping(Scene* scene /*...*/);
static void computeLighting(Scene* scene /*...*/);
static void renderCaustics(Scene* scene /*...*/);

static void drawEntity(const Entity* entity, RenderSystem::MVPset &MVP, ShaderLibrary &shaderlib);
static void drawFlock(const Entity* entity);

static void drawGeometry(const Geometry &geomcomp, RenderSystem::MVPset &MVP);
static void drawAntennae(/*...*/);
static void drawLamp(/*...*/);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // RenderSystem functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void RenderSystem::init(ApplicationState &appstate){
  glfwGetFramebufferSize(appstate.window, &w_width, &w_height);
  glViewport(0, 0, w_width, w_height);
  MVP.P = MatrixStack(glm::perspective(45.0, static_cast<double>(w_width)/w_height, .01, 100.0));
  postProcessor = new PostProcessor(appstate.window);
}

void RenderSystem::render(ApplicationState &appstate, GameState &gstate, double elapsedTime){

  // Added this so it isn't just black screen. 
  glClearColor( .18, .20, .22, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  MVP.M.loadIdentity();
  MVP.V = MatrixStack(updateView(gstate.activeScene->camera));

  // This is a quick hack, will be replaced by 
  {
    appstate.resources.shaderlib.makeActive("blinn-phong");
    Program &prog = appstate.resources.shaderlib.getActive();
    glUniform3f(prog.getUniform("sunDir"), 0.5, -1.0, 1.0);
    glUniform3f(prog.getUniform("sunCol"), .7, .7, .65);
  }

  computeLighting();
  drawBackground();
  drawAngler();
  drawEntities(gstate.activeScene, MVP, appstate.resources.shaderlib);
  drawEffects();
  postProcess();
}

void RenderSystem::onResize(GLFWwindow *window, int width, int height){
  w_width = width;
  w_height = height;
  glViewport(0, 0, width, height);
  postProcessor->resize();
}


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// Static Functions and Helpers
// It may be possible to move some parts of this into one or more libraries. 
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static glm::mat4 updateView(Component* cmpnt){
  Camera* camera;
  if((camera = dynamic_cast<Camera*>(cmpnt))){
    return(camera->getView());
  }
  return(glm::mat4());
}

static void drawEntities(Scene* scene, RenderSystem::MVPset &MVP, ShaderLibrary &shaderlib){
  for(pair<const Entity*, Entity*> entpair : scene->entities){
    drawEntity(entpair.second, MVP, shaderlib);
  }
}

static void drawEntity(const Entity* entity, RenderSystem::MVPset &MVP, ShaderLibrary &shaderlib){
  SolidMesh* mesh = NULL;
  Pose* pose = NULL;
  for(Component *cmpnt : entity->components){
    GATHER_SINGLE_COMPONENT(mesh, SolidMesh*, cmpnt);
    GATHER_SINGLE_COMPONENT(pose, Pose*, cmpnt);
    
    if(mesh && pose){
      MVP.M.multMatrix(pose->getAffineMatrix());
      for(Geometry &geo : mesh->geometries){
        shaderlib.fastActivate(geo.material.shader);
        drawGeometry(geo, MVP);
      }
      return;
    }
  }
  if(mesh){
    for(Geometry &geo : mesh->geometries){
      shaderlib.fastActivate(geo.material.shader);
      drawGeometry(geo, MVP);
    }
  }
}

static void drawGeometry(const Geometry &geomcomp, RenderSystem::MVPset &MVP){
  int h_pos, h_nor, h_tex;
  h_pos = h_nor = h_tex = -1;

  Program* shader = geomcomp.material.shader;
  geomcomp.material.apply();

  glUniformMatrix4fv(shader->getUniform("M"), 1, GL_FALSE, value_ptr(MVP.M.topMatrix()));
  glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, value_ptr(MVP.V.topMatrix()));
  glUniformMatrix4fv(shader->getUniform("P"), 1, GL_FALSE, value_ptr(MVP.P.topMatrix()));

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


static void computeLighting(/*...*/){}
static void drawBackground(/*...*/){}
static void drawAngler(/*...*/){}
static void drawEffects(/*...*/){}
static void postProcess(){
	// TODO: pass texture as parameter
	postProcessor->doPostProcessing(0);
}
static void shadowMapping(Scene* scene /*...*/){}
static void computeLighting(Scene* scene /*...*/){}
static void renderCaustics(Scene* scene /*...*/){}
static void drawFlock(const Entity* entity){}
static void drawAntennae(/*...*/){}
static void drawLamp(/*...*/){}

#pragma GCC diagnostic pop