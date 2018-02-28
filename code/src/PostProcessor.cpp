#include "PostProcessor.h"

static void initQuad(GLuint &quadVAO, GLuint &quadVBO);

static void createFBO(int w_width, int w_height, GLuint& fb, GLuint& tex, GLenum filter, GLenum wrap);

static void applyBrightFilter(GLuint colorTex, Program* brightFilterProg);
static void applyHBlur(GLuint brightTex, Program* hBlurProg);
static void applyVBlur(GLuint brightTex, Program* vBlurProg);
static void applyCombine(GLuint colorTex, GLuint brightTex, Program* combineProg);


void PostProcessor::init(int _w_width, int _w_height, ShaderLibrary* _shaderlib)
{
  shaderlib = _shaderlib;
  w_width = _w_width;
  w_height = _w_height;

  initQuad(quadVAO, quadVBO);

  //create three frame buffer objects to toggle between
  glGenFramebuffers(POSTPROCESSOR_BUFFER_COUNT, frameBuf);
  glGenTextures(POSTPROCESSOR_BUFFER_COUNT, texBuf);
  glGenRenderbuffers(1, &depthBuf);

  for(int i = 0; i < POSTPROCESSOR_BUFFER_COUNT; i++){
    createFBO(w_width, w_height, frameBuf[i], texBuf[i], GL_LINEAR, GL_CLAMP_TO_EDGE);
  }
}

void PostProcessor::doPostProcessing(GLuint texture, GLuint output)
{
  int lastout;
  lastout = processBloom(texture, -1);
  lastout = processDepthOfField(texBuf[lastout], -1);
  lastout = runFXAA(texBuf[lastout], output);
}

int PostProcessor::processBloom(GLuint texture, int output)
{
  int fboID1 = nextFBO();
  int fboID2 = nextFBO();
  // Bright filter
  glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[fboID1]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  applyBrightFilter(texture, shaderlib->getPtr("bloom_brightFilter"));

  for (int i = 0; i < BLUR_AMOUNT; i++)
  {
    // Horizontal blur
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[fboID2]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    applyHBlur(texBuf[fboID1], shaderlib->getPtr("bloom_horizontalBlur"));
    
    // Vertical blur
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuf[fboID1]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    applyVBlur(texBuf[fboID2], shaderlib->getPtr("bloom_verticalBlur"));
  }

  // Combine
  glBindFramebuffer(GL_FRAMEBUFFER, output >= 0 ? (GLuint)output : frameBuf[fboID2]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  applyCombine(texture, texBuf[fboID1], shaderlib->getPtr("bloom_combine"));
  ASSERT_NO_GLERR();
  return(fboID2);
}

int PostProcessor::runFXAA(GLuint texture, int output){
  int fboID = output >= 0 ? (GLuint)output : nextFBO();
  glBindFramebuffer(GL_FRAMEBUFFER, output >= 0 ? (GLuint)output : frameBuf[fboID]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shaderlib->makeActive("FXAA");
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glUniform1i(shaderlib->getActive().getUniform("pixtex"), 0);
  glUniform2f(shaderlib->getActive().getUniform("resolution"), static_cast<float>(w_width), static_cast<float>(w_height));
  
  drawFSQuad();
  ASSERT_NO_GLERR();
  return fboID;
}

int PostProcessor::processDepthOfField(GLuint texture, int output)
{
  int fboID = output >= 0 ? (GLuint)output : nextFBO();
  glBindFramebuffer(GL_FRAMEBUFFER, output >= 0 ? (GLuint)output : frameBuf[fboID]);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // TODO: change to be depth of field
  shaderlib->makeActive("FXAA");
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glUniform1i(shaderlib->getActive().getUniform("pixtex"), 0);
  glUniform2f(shaderlib->getActive().getUniform("resolution"), static_cast<float>(w_width), static_cast<float>(w_height));
 
  drawFSQuad();
  ASSERT_NO_GLERR();
  return fboID;
}

void PostProcessor::drawFSQuad(){
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);
}

static void initQuad(GLuint &quadVAO, GLuint &quadVBO) {
  float quadVertices[] = {
    // positions
    -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
  };
  // setup plane VAO
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

static void createFBO(int w_width, int w_height, GLuint& fb, GLuint& tex, GLenum filter, GLenum wrap)
{
  //set up framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, fb);
  //set up texture
  glBindTexture(GL_TEXTURE_2D, tex);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w_width, w_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cout << "Error setting up frame buffer - exiting" << std::endl;
    exit(0);
  }
}

static void applyBrightFilter(GLuint colorTex, Program* brightFilterProg)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, colorTex);

  brightFilterProg->bind();
  glUniform1i(brightFilterProg->getUniform("colorTexture"), 0);
  PostProcessor::drawFSQuad();
  brightFilterProg->unbind();
}

static void applyHBlur(GLuint brightTex, Program* hBlurProg)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, brightTex);

  hBlurProg->bind();
  glUniform1i(hBlurProg->getUniform("brightTexture"), 0);
  PostProcessor::drawFSQuad();
  hBlurProg->unbind();
}

static void applyVBlur(GLuint brightTex, Program* vBlurProg)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, brightTex);

  vBlurProg->bind();
  glUniform1i(vBlurProg->getUniform("brightTexture"), 0);
  PostProcessor::drawFSQuad();
  vBlurProg->unbind();
}

static void applyCombine(GLuint colorTex, GLuint brightTex, Program* combineProg)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, colorTex);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, brightTex);

  combineProg->bind();
  glUniform1i(combineProg->getUniform("colorTexture"), 0);
  glUniform1i(combineProg->getUniform("brightTexture"), 1);
  PostProcessor::drawFSQuad();
  combineProg->unbind();
}