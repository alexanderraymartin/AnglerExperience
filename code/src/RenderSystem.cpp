#include "RenderSystem.hpp"
#include <common.h>
#include <GLSL.h>
#include <Program.h>
#include "core.h"
#include "GameState.hpp"
#include "Entity.hpp"

#include "PostProcessor.h"
#include "components/SimpleComponents.hpp"
#include "components/AnimationComponents.hpp"
#include "components/Geometry.hpp"
#include "LightingComponents.hpp"


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Forward Declarations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static void prepareDeferred(GLuint gbuffer);
static void initDeferredBuffers(int width, int height, RenderSystem::Buffers &buffers);
static void initOutputFBO(GLuint* render_out_FBO, GLuint* render_out_color, int w_width, int w_height, GLenum filter);
static void bindGBuf(RenderSystem::Buffers &buffers, Program* shader);

static void postProcess(/*...*/);

static void createGBufAttachment(int width, int height, vector<unsigned int> &buffers, unsigned int channel_type, unsigned int channels, unsigned int type);
static void setGBufAttachment(RenderSystem::Buffers &buffers);
static void setGBufDepth(int width, int height, RenderSystem::Buffers &buffers);

static void drawGeometry(const Geometry &geomcomp, RenderSystem::MVPset &MVP, Program* shader);
static void drawGeometry(const Geometry &geomcomp, const Geometry *geomcomp2, 
  RenderSystem::MVPset &MVP, Program* shader, double interp);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // RenderSystem functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void RenderSystem::init(ApplicationState &appstate){
  glfwGetFramebufferSize(appstate.window, &w_width, &w_height);
  glViewport(0, 0, w_width, w_height);

  shaderlib = &appstate.resources.shaderlib;

  deferred_export = shaderlib->getPtr("deferred-export");
  deferred_uber = shaderlib->getPtr("deferred-uber");
  deferred_shadow = shaderlib->getPtr("deferred-shadow");
  deferred_shadow->setVerbose(false);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  initDeferredBuffers(w_width, w_height, deferred_buffers);

  initOutputFBO(&render_out_FBO, &render_out_color, w_width, w_height, GL_LINEAR);

  initDepthUniforms(1, 10);
  initCaustics();
  initShadowMap(w_width, w_height);

  PostProcessor::init(w_width, w_height, shaderlib);
}

void RenderSystem::render(ApplicationState &appstate, GameState &gstate, double elapsedTime){
  Camera* camera;
  if(!(camera = gstate.activeScene->camera)){
    return;
  }
  
  MVP.P = MatrixStack(camera->getPerspective(static_cast<double>(w_width)/w_height));
  MVP.M.loadIdentity();
  MVP.V = MatrixStack(camera->getView());

  prepareDeferred(deferred_buffers.gBuffer);
  drawEntities(gstate.activeScene, deferred_export);

  updateLighting(gstate.activeScene);
  updateDepthUniforms();

  prepareDeferred(shadowFramebuffer);
  drawEntities(gstate.activeScene, deferred_shadow);

  updateShadowMap();
  updateCaustic();
  applyShading(gstate.activeScene, *shaderlib);

  PostProcessor::doPostProcessing(render_out_color, 0);
}

// This function is aweful and I hate it.
void RenderSystem::updateLighting(Scene* scene){
  const static char* colbase = "pointLights.colors[%u]";
  const static char* locbase = "pointLights.positions[%u]";
  size_t baselen[2] = {strlen(colbase)+9, strlen(locbase)+9};
  static char* colid = NULL; 
  colid = (colid == NULL) ? new char[baselen[0]+1] : colid;
  static char* locid = NULL; 
  locid = (locid == NULL) ? new char[baselen[1]+1] : locid;

  UINT lightnum = 0;

  shaderlib->fastActivate(deferred_uber);
  for(pair<const Entity*, Entity*> entpair : scene->entities){
    SunLight* suncomp = NULL;
    PointLight* ptcomp = NULL;
    Pose* pose = NULL;
    // Locations for point lights so we only fetch once
    for(Component *cmpnt : entpair.second->components){
      GATHER_SINGLE_COMPONENT(pose, Pose*, cmpnt);
      GATHER_SINGLE_COMPONENT(ptcomp, PointLight*, cmpnt);
      // fprintf(stderr, "%p and %p\n", suncomp, pose);
      if(CATCH_COMPONENT(suncomp, SunLight*, cmpnt)){
        SunLight* sun = static_cast<SunLight*>(cmpnt);
        glUniform3fv(deferred_uber->getUniform("sunCol"), 1, value_ptr(sun->color));
        glUniform3fv(deferred_uber->getUniform("sunDir"), 1, value_ptr(sun->direction));

        vec3 lightPos = vec3(0, 10, 0);
        depthSet.lightView.loadIdentity();
        depthSet.lightView.lookAt(lightPos, lightPos + sun->direction, vec3(0,1,0));

      }else if(ptcomp && pose){
        // This would be a great place to implement DOD
        snprintf(colid, baselen[0], colbase, lightnum);
        snprintf(locid, baselen[1], locbase, lightnum);
        lightnum++;
        glUniform3fv(deferred_uber->getUniform(colid), 1, value_ptr(ptcomp->color));
        glUniform3fv(deferred_uber->getUniform(locid), 1, value_ptr(pose->loc));
        pose = NULL;
      }
    }
    glUniform1i(deferred_uber->getUniform("numLights"), lightnum);
  }
}

void RenderSystem::applyShading(Scene* scene, ShaderLibrary &shaderlib){
  glBindFramebuffer(GL_FRAMEBUFFER, render_out_FBO);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shaderlib.fastActivate(deferred_uber);
  bindGBuf(deferred_buffers, deferred_uber);
  PostProcessor::drawFSQuad();
}

void RenderSystem::onResize(GLFWwindow *window, int width, int height){
  w_width = width;
  w_height = height;
  glViewport(0, 0, width, height);
  PostProcessor::resize(width, height);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Static Functions and Helpers
// It may be possible to move some parts of this into one or more libraries. 
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void RenderSystem::drawEntities(Scene* scene, Program* shader){
  for(pair<const Entity*, Entity*> entpair : scene->entities){
    drawEntity(entpair.second, shader, true);
  }
  while (!(renderq.empty())) {
    drawEntity(renderq.front(), shader, false);
    renderq.pop();
  }
}

void RenderSystem::drawEntity(const Entity* entity, Program* shader, bool buildq){
  SolidMesh* mesh = NULL, *mesh2;
  Pose* pose = NULL;
  AnimatableMesh* anim = NULL;
  for(Component *cmpnt : entity->components){
    GATHER_SINGLE_COMPONENT(mesh, SolidMesh*, cmpnt);
    GATHER_SINGLE_COMPONENT(pose, Pose*, cmpnt);
    GATHER_SINGLE_COMPONENT(anim, AnimatableMesh*, cmpnt);
    
    if (anim && pose) {
      if (buildq) {
        renderq.push(entity);
        return;
      }
      MVP.M.pushMatrix();
      MVP.M.multMatrix(pose->getAffineMatrix());
      mesh = anim->getCurrentMesh();
      mesh2 = anim->getNextMesh();
      for (int i = 0; i < mesh->geometries.size(); i++) {
        drawGeometry(mesh->geometries[i], &mesh2->geometries[i],
          MVP, shader, anim->dtLastKeyFrame / anim->timeForKeyFrame());
      }
      MVP.M.popMatrix();
      mesh = mesh2 = NULL;
      pose = NULL;
      return;
    }
    if(mesh && pose){
      MVP.M.pushMatrix();
      MVP.M.multMatrix(pose->getAffineMatrix());
      for(Geometry &geo : mesh->geometries){
        shaderlib->fastActivate(shader);
        drawGeometry(geo, MVP, shader);
      }
      MVP.M.popMatrix();
      mesh = NULL;
      pose = NULL;
      return;
    }
  }
  if(mesh){
    for(Geometry &geo : mesh->geometries){
      shaderlib->fastActivate(shader);
      drawGeometry(geo, MVP, shader);
    }
  }
}

static void drawGeometry(const Geometry &geomcomp, RenderSystem::MVPset &MVP, Program* shader){
  drawGeometry(geomcomp, NULL, MVP, shader, 0);
}

static void drawGeometry(const Geometry &geomcomp, const Geometry *geomcomp2, 
  RenderSystem::MVPset &MVP, Program* shader, double interp){
  int h_pos, h_nor, h_tex;
  int h_pos2, h_nor2, h_tex2;
  h_pos = h_nor = h_tex = -1;
  h_pos2 = h_nor2 = h_tex2 = -1;

  geomcomp.material.apply(shader);

  glUniformMatrix4fv(shader->getUniform("M"), 1, GL_FALSE, value_ptr(MVP.M.topMatrix()));
  glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, value_ptr(MVP.V.topMatrix()));
  glUniformMatrix4fv(shader->getUniform("P"), 1, GL_FALSE, value_ptr(MVP.P.topMatrix()));
  glUniform1f(shader->getUniform("interp"), interp);

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

  if (geomcomp2) {
    // Bind position buffer
    h_pos2 = shader->getAttribute("vertPos2");
    GLSL::enableVertexAttribArray(h_pos2);
    glBindBuffer(GL_ARRAY_BUFFER, geomcomp2->posBufID);
    glVertexAttribPointer(h_pos2, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);

    // Bind normal buffer
    h_nor2 = shader->getAttribute("vertNor2");
    if(h_nor2 != -1 && geomcomp2->norBufID != 0) {
      GLSL::enableVertexAttribArray(h_nor2);
      glBindBuffer(GL_ARRAY_BUFFER, geomcomp2->norBufID);
      glVertexAttribPointer(h_nor2, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
    }
    
    if (geomcomp2->texBufID != 0) {  
      // Bind texcoords buffer
      h_tex2 = shader->getAttribute("vertTex2");
      if(h_tex2 != -1 && geomcomp2->texBufID != 0) {
        GLSL::enableVertexAttribArray(h_tex2);
        glBindBuffer(GL_ARRAY_BUFFER, geomcomp2->texBufID);
        glVertexAttribPointer(h_tex2, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
      }
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

  if (geomcomp2) {
    if(h_tex2 != -1) {
      GLSL::disableVertexAttribArray(h_tex2);
    }
    if(h_nor2 != -1) {
      GLSL::disableVertexAttribArray(h_nor2);
    }
    GLSL::disableVertexAttribArray(h_pos2);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static void prepareDeferred(GLuint gbuffer){
  glBindFramebuffer(GL_FRAMEBUFFER, gbuffer);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void bindGBuf(RenderSystem::Buffers &buffers, Program* shader) {
  for (int i = 0; i < buffers.buffers.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, buffers.buffers.at(i));
  }
  glUniform1i(shader->getUniform("gPosition"), 0);
  glUniform1i(shader->getUniform("gNormal"), 1);
  glUniform1i(shader->getUniform("gAlbedo"), 2);
  glUniform1i(shader->getUniform("gSpecular"), 3);

}

static void setGBufAttachment(RenderSystem::Buffers &buffers) {
  vector<unsigned int> attachments;
  for (int i = 0; i < buffers.buffers.size(); i++) {
    //size - i because we want the list to be [0,1,2] not [2,1,0]
    attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
  }
  glDrawBuffers(buffers.buffers.size(), attachments.data());
}

static void createGBufAttachment(int width, int height, vector<unsigned int> &buffers, unsigned int channel_type, unsigned int channels, unsigned int type) {
  unsigned int buffer;
  glGenTextures(1, &buffer);
  glBindTexture(GL_TEXTURE_2D, buffer);
  glTexImage2D(GL_TEXTURE_2D, 0, channel_type, width, height, 0, channels, type, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + buffers.size() , GL_TEXTURE_2D, buffer, 0);
  buffers.push_back(buffer);
}

static void setGBufDepth(int width, int height, RenderSystem::Buffers &buffers) {
  (glGenRenderbuffers(1, &buffers.depthBuffer));
  (glBindRenderbuffer(GL_RENDERBUFFER, buffers.depthBuffer));
  (glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height));
  (glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buffers.depthBuffer));
}

static void initDeferredBuffers(int width, int height, RenderSystem::Buffers &buffers){
  //set up to render to an intermediary buffer
  glGenFramebuffers(1, &(buffers.gBuffer));
  glBindFramebuffer(GL_FRAMEBUFFER, buffers.gBuffer);
  //32 bit floats may not be supported on some systems.
  createGBufAttachment(width, height, buffers.buffers, GL_RGB16F, GL_RGB, GL_FLOAT);     //position buffer
  createGBufAttachment(width, height, buffers.buffers, GL_RGB16F, GL_RGB, GL_FLOAT);     //normal buffer
  createGBufAttachment(width, height, buffers.buffers, GL_RGBA, GL_RGBA, GL_UNSIGNED_INT); //color/emit buffer
  createGBufAttachment(width, height, buffers.buffers, GL_RGBA, GL_RGBA, GL_UNSIGNED_INT); // Specular/shine buffer
  setGBufAttachment(buffers);
  setGBufDepth(width, height, buffers);                            //Depth buffer
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    fprintf(stderr, "Framebuffer not complete!\n");
}

static void initOutputFBO(GLuint* outFBO, GLuint* outColor, int w_width, int w_height, GLenum filter) {
  glGenFramebuffers(1, outFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, *outFBO);
  glGenTextures(1, outColor);
  glBindTexture(GL_TEXTURE_2D, *outColor);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w_width, w_height, 0, GL_RGBA, GL_UNSIGNED_INT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *outColor, 0);
  glDrawBuffer(GL_COLOR_ATTACHMENT0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    fprintf(stderr, "Framebuffer not complete!\n");
  }

  ASSERT_NO_GLERR();
}

void RenderSystem::initCaustics() {

  causticDir = "" STRIFY(ASSET_DIR) "/caustics";

  for (int i = 0; i < CAUSTIC_COUNT; i++) {
      char filename[80];
      sprintf(filename, "/caustic%02d.jpg", i + 1);

      caustics[i] = make_shared<Texture>();
      caustics[i]->setFilename(causticDir + filename);
      caustics[i]->init();
      caustics[i]->setUnit(deferred_buffers.buffers.size() + i);
      caustics[i]->setWrapModes(GL_REPEAT, GL_REPEAT);
  }
}

void RenderSystem::initDepthUniforms(double causticSize, double shadowSize) {
  depthSet.causticOrtho.ortho(-causticSize, causticSize, -causticSize, causticSize, 0.01, 100.0);

  depthSet.shadowOrtho.ortho(-shadowSize, shadowSize, -shadowSize, shadowSize, 0.01, 100.0);

  depthSet.bias.loadIdentity();
  depthSet.bias.translate(vec3(0.5, 0.5, 0.5));
  depthSet.bias.scale(0.5);
}

void RenderSystem::updateDepthUniforms() {
  mat4 causticMatrix = depthSet.bias.topMatrix() * depthSet.causticOrtho.topMatrix() * depthSet.lightView.topMatrix();
  mat4 shadowMatrix = depthSet.bias.topMatrix() * depthSet.shadowOrtho.topMatrix() * depthSet.lightView.topMatrix();
  mat4 depthMatrix = depthSet.shadowOrtho.topMatrix() * depthSet.lightView.topMatrix();

  deferred_uber->bind();

  glUniformMatrix4fv(deferred_uber->getUniform("causticMatrix"), 1, GL_FALSE, value_ptr(causticMatrix));
  glUniformMatrix4fv(deferred_uber->getUniform("shadowMatrix"), 1, GL_FALSE, value_ptr(shadowMatrix));

  deferred_uber->unbind();

  deferred_shadow->bind();

  glUniformMatrix4fv(deferred_shadow->getUniform("depthMatrix"), 1, GL_FALSE, value_ptr(depthMatrix));

  deferred_shadow->unbind();
}

void RenderSystem::updateCaustic() {
  deferred_uber->bind();

  caustics[currCaustic]->bind(deferred_uber->getUniform("caustic"));
  currCaustic = ++currCaustic >= CAUSTIC_COUNT ? 0 : currCaustic;

  deferred_uber->unbind();
}

void RenderSystem::initShadowMap(int width, int height) {
  glGenFramebuffers(1, &shadowFramebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);

  // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
  glGenTextures(1, &shadowTexture);
  glBindTexture(GL_TEXTURE_2D, shadowTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  GLfloat color[4]={1,1,1,1};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTexture, 0);

  glDrawBuffer(GL_NONE); // No color buffer is drawn to.

  // Always check that our framebuffer is ok
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    exit(0);
}

void RenderSystem::updateShadowMap() {
  deferred_uber->bind();

  glActiveTexture(GL_TEXTURE0 + (deferred_buffers.buffers.size() + CAUSTIC_COUNT));
  glBindTexture(GL_TEXTURE_2D, shadowTexture);
  glUniform1i(deferred_uber->getUniform("shadowMap"), (deferred_buffers.buffers.size() + CAUSTIC_COUNT));

  deferred_uber->unbind();
}
