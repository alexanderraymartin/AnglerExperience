#include "RenderSystem.hpp"
#include <common.h>
#include <GLSL.h>
#include <Program.h>
#include "core.h"
#include "GameState.hpp"
#include "Entity.hpp"

#include "components/SimpleComponents.hpp"
#include "components/Geometry.hpp"
#include "components/AnimationComponents.hpp"

// Disable complaints about all the placeholder functions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Forward Declarations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

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

static void drawGeometry(const Geometry &geom1, const Geometry *geom2, RenderSystem::MVPset &MVP, Program &shader, double interp);
static void drawGeometry(const Geometry &geom1, RenderSystem::MVPset &MVP, Program &shader);
static void drawAntennae(/*...*/);
static void drawLamp(/*...*/);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // RenderSystem functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void RenderSystem::init(ApplicationState &appstate){
  glfwGetFramebufferSize(appstate.window, &w_width, &w_height);
  glViewport(0, 0, w_width, w_height);
  MVP.P = MatrixStack(glm::perspective(45.0, static_cast<double>(w_width)/w_height, .01, 100.0));
}

void RenderSystem::render(ApplicationState &appstate, GameState &gstate, double elapsedTime){

  // Added this so it isn't just black screen. 
  glClearColor( .18, .20, .22, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  MVP.M.loadIdentity();
  MVP.V = MatrixStack(updateView(gstate.activeScene->camera));

  // This is a quick hack, will be removed once a materials infrastructure is in place
  {
    appstate.resources.shaderlib.makeActive("blinn-phong");
    Program &prog = appstate.resources.shaderlib.getActive();
    glUniform3f(prog.getUniform("sunDir"), 0.5, -1.0, 1.0);
    glUniform3f(prog.getUniform("sunCol"), .7, .7, .65);
    glUniform3f(prog.getUniform("matAmb"), .1, .1, .1);
    glUniform3f(prog.getUniform("matDif"), .7, .7, .7);
    glUniform3f(prog.getUniform("matSpec"), .4, .4, .4);
    glUniform1f(prog.getUniform("shine"), 9.0);
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
  SolidMesh* mesh = NULL, *mesh2;
  Pose* pose = NULL;
  AnimatableMesh* anim = NULL;
  for(Component *cmpnt : entity->components){
    mesh = dynamic_cast<SolidMesh*>(cmpnt) == NULL ? mesh : static_cast<SolidMesh*>(cmpnt);
    pose = dynamic_cast<Pose*>(cmpnt) == NULL ? pose : static_cast<Pose*>(cmpnt);
    anim = dynamic_cast<AnimatableMesh*>(cmpnt) == NULL ? anim : static_cast<AnimatableMesh*>(cmpnt);

    if (anim && pose) {
      MVP.M.multMatrix(pose->getAffineMatrix());
      mesh = anim->getCurrentMesh();
      mesh2 = anim->getNextMesh();
      for (int i = 0; i < mesh->geometries.size(); i++) {
        drawGeometry(mesh->geometries[i], &mesh2->geometries[i],
          MVP, shaderlib.getActive(), anim->dtLastKeyFrame / anim->timePerKeyFrame);
      }
      return;
    }
    
    if(mesh && pose){
      MVP.M.multMatrix(pose->getAffineMatrix());
      for(Geometry &geo : mesh->geometries){
        drawGeometry(geo, MVP, shaderlib.getActive());
      }
      return;
    }
  }
  if(mesh){
    for(Geometry &geo : mesh->geometries){
        drawGeometry(geo, MVP, shaderlib.getActive());
    }
  }
}

static void drawGeometry(const Geometry &geom1, RenderSystem::MVPset &MPV, Program &shader) {
  drawGeometry(geom1, NULL, MPV, shader, 0);
}

static void drawGeometry(const Geometry &geom1, const Geometry *geom2,
  RenderSystem::MVPset &MVP, Program &shader, double interp){
  int h_pos, h_nor, h_tex;
  int h_pos2, h_nor2, h_tex2;
  h_pos = h_nor = h_tex = -1;

  glUniformMatrix4fv(shader.getUniform("M"), 1, GL_FALSE, value_ptr(MVP.M.topMatrix()));
  glUniformMatrix4fv(shader.getUniform("V"), 1, GL_FALSE, value_ptr(MVP.V.topMatrix()));
  glUniformMatrix4fv(shader.getUniform("P"), 1, GL_FALSE, value_ptr(MVP.P.topMatrix()));
  if (geom2) {
    glUniform1f(shader.getUniform("interp"), interp);
  }

  glBindVertexArray(geom1.vaoID);

  // Bind position buffer
  h_pos = shader.getAttribute("vertPos");
  GLSL::enableVertexAttribArray(h_pos);
  glBindBuffer(GL_ARRAY_BUFFER, geom1.posBufID);
  glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  
  // Bind normal buffer
  h_nor = shader.getAttribute("vertNor");
  if(h_nor != -1 && geom1.norBufID != 0) {
    GLSL::enableVertexAttribArray(h_nor);
    glBindBuffer(GL_ARRAY_BUFFER, geom1.norBufID);
    glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  }

  if (geom1.texBufID != 0) {  
    // Bind texcoords buffer
    h_tex = shader.getAttribute("vertTex");
    if(h_tex != -1 && geom1.texBufID != 0) {
      GLSL::enableVertexAttribArray(h_tex);
      glBindBuffer(GL_ARRAY_BUFFER, geom1.texBufID);
      glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    }
  }

  if (geom2) {
    // Bind position buffer
    h_pos2 = shader.getAttribute("vertPos2");
    GLSL::enableVertexAttribArray(h_pos2);
    glBindBuffer(GL_ARRAY_BUFFER, geom2->posBufID);
    glVertexAttribPointer(h_pos2, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
  
    // Bind normal buffer
    h_nor2 = shader.getAttribute("vertNor2");
    if(h_nor2 != -1 && geom2->norBufID != 0) {
      GLSL::enableVertexAttribArray(h_nor2);
      glBindBuffer(GL_ARRAY_BUFFER, geom2->norBufID);
      glVertexAttribPointer(h_nor2, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    }

    if (geom2->texBufID != 0) {  
      // Bind texcoords buffer
      h_tex2 = shader.getAttribute("vertTex2");
      if(h_tex2 != -1 && geom2->texBufID != 0) {
        GLSL::enableVertexAttribArray(h_tex2);
        glBindBuffer(GL_ARRAY_BUFFER, geom2->texBufID);
        glVertexAttribPointer(h_tex2, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
      }
    }
  }
  
  // Bind element buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geom1.eleBufID);
  
  // Draw
  glDrawElements(GL_TRIANGLES, (int)geom1.eleBuf->size(), GL_UNSIGNED_INT, (const void *)0);
  
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
static void postProcess(/*...*/){}
static void shadowMapping(Scene* scene /*...*/){}
static void computeLighting(Scene* scene /*...*/){}
static void renderCaustics(Scene* scene /*...*/){}
static void drawFlock(const Entity* entity){}
static void drawAntennae(/*...*/){}
static void drawLamp(/*...*/){}

#pragma GCC diagnostic pop
