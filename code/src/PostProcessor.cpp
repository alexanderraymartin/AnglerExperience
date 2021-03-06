#include "PostProcessor.h"

static void initQuad(GLuint &quadVAO, GLuint &quadVBO);

static void createFBO(int w_width, int w_height, GLuint& fb, GLuint& tex, GLenum filter, GLenum wrap);

static void applyDepthOfFieldCombine(GLuint tex, GLuint blurredTex, GLuint depthBuffer, GLuint fbo, Program* combineProg);
static void applyBlur(GLuint tex, GLuint fbo, Program* prog);
static void applyBrightFilter(GLuint tex, GLuint fbo, Program* prog);
static void applyBloomCombine(GLuint tex, GLuint brightTex, GLuint fbo, Program* combineProg);


void PostProcessor::init(int _w_width, int _w_height, ShaderLibrary* _shaderlib)
{
  shaderlib = _shaderlib;
  w_width = _w_width;
  w_height = _w_height;

  initQuad(quadVAO, quadVBO);

  // Create full resolution buffers
  glGenFramebuffers(POSTPROCESSOR_BUFFER_COUNT, frameBuf);
  glGenTextures(POSTPROCESSOR_BUFFER_COUNT, texBuf);

  for(int i = 0; i < POSTPROCESSOR_BUFFER_COUNT; i++){
    createFBO(w_width, w_height, frameBuf[i], texBuf[i], GL_LINEAR, GL_CLAMP_TO_EDGE);
  }

  // Create low resolution buffers
  glGenFramebuffers(2, lowResFrameBuf);
  glGenTextures(2, lowResTexBuf);

  for (int i = 0; i < POSTPROCESSOR_LOW_RES_BUFFER_COUNT; i++) {
    createFBO(w_width / LOW_RES_FBO_SCALE, w_height / LOW_RES_FBO_SCALE, lowResFrameBuf[i], lowResTexBuf[i], GL_LINEAR, GL_CLAMP_TO_EDGE);
  }
}

void PostProcessor::doPostProcessing(GLuint texture, GLuint depthBuffer)
{
  int lastout;

  if (HAS_POST_PROCESSING) {
    lastout = runFXAA(texture, false);
    lastout = processBloom(texBuf[lastout], false);
    lastout = processDepthOfField(texBuf[lastout], depthBuffer, true);
  }
  else {
	passThroughShader(texture);
  }
}

void PostProcessor::passThroughShader(GLuint texture)
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  
  Program * prog = shaderlib->getPtr("passthru");
  prog->bind();
  glUniform1i(prog->getUniform("pixtex"), 0);
  PostProcessor::drawFSQuad();
  prog->unbind();
}


int PostProcessor::processBloom(GLuint texture, bool isLast)
{
  int fboID1 = nextLowResFBO();
  int fboID2 = nextLowResFBO();
  int fboID3 = nextFBO();
  
  // Bright filter
  glViewport(0, 0, w_width / LOW_RES_FBO_SCALE, w_height / LOW_RES_FBO_SCALE);
  applyBrightFilter(texture, lowResFrameBuf[fboID1], shaderlib->getPtr("bloom_brightFilter"));

  for (int i = 0; i < BLOOM_BLUR_AMOUNT; i++)
  {
    // Horizontal blur
	applyBlur(lowResTexBuf[fboID1], lowResFrameBuf[fboID2], shaderlib->getPtr("bloom_horizontalBlur"));
    
    // Vertical blur
	applyBlur(lowResTexBuf[fboID2], lowResFrameBuf[fboID1], shaderlib->getPtr("bloom_verticalBlur"));
  }

  // Combine
  glViewport(0, 0, w_width, w_height);
  applyBloomCombine(texture, lowResTexBuf[fboID1], isLast ? 0 : frameBuf[fboID3], shaderlib->getPtr("bloom_combine"));
  ASSERT_NO_GLERR();
  return fboID3;
}

int PostProcessor::processDepthOfField(GLuint texture, GLuint depthBuffer, bool isLast)
{
  int fboID1 = nextLowResFBO();
  int fboID2 = nextLowResFBO();
  int fboID3 = nextFBO();
  
  // Horizontal blur 1st pass
  glViewport(0, 0, w_width / LOW_RES_FBO_SCALE, w_height / LOW_RES_FBO_SCALE);
  applyBlur(texture, lowResFrameBuf[fboID1], shaderlib->getPtr("depthOfField_horizontalBlur"));
  
  // Vertical blur 1st pass
  applyBlur(lowResTexBuf[fboID1], lowResFrameBuf[fboID2], shaderlib->getPtr("depthOfField_verticalBlur"));
  
  for (int i = 0; i < DEPTH_OF_FIELD_BLUR_AMOUNT - 1; i++)
  {
    // Horizontal blur mutli-pass
    applyBlur(lowResTexBuf[fboID2], lowResFrameBuf[fboID1], shaderlib->getPtr("depthOfField_horizontalBlur"));
    
    // Vertical blur mutli-pass
    applyBlur(lowResTexBuf[fboID1], lowResFrameBuf[fboID2], shaderlib->getPtr("depthOfField_verticalBlur"));
  }
  
  // Combine blur with full res texture
  glViewport(0, 0, w_width, w_height);
  applyDepthOfFieldCombine(texture, lowResTexBuf[fboID2], depthBuffer, isLast ? 0 : frameBuf[fboID3], shaderlib->getPtr("depthOfField_depthOfFieldCombine"));
  
  ASSERT_NO_GLERR();
  return fboID3;
}

int PostProcessor::runFXAA(GLuint texture, bool isLast)
{
  int fboID = isLast ? 0 : nextFBO();
  glBindFramebuffer(GL_FRAMEBUFFER, isLast ? 0 : frameBuf[fboID]);
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

/////////////////////////////////////////////
/* Depth of Field */
static void applyDepthOfFieldCombine(GLuint tex, GLuint blurredTex, GLuint depthBuffer, GLuint fbo, Program* prog)
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, blurredTex);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, depthBuffer);
  
  prog->bind();
  glUniform1i(prog->getUniform("tex"), 0);
  glUniform1i(prog->getUniform("blurredTex"), 1);
  glUniform1i(prog->getUniform("depthBufTex"), 2);
  PostProcessor::drawFSQuad();
  prog->unbind();
}

/////////////////////////////////////////////
/* Bloom */
void applyBlur(GLuint tex, GLuint fbo, Program * prog)
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  
  prog->bind();
  glUniform1i(prog->getUniform("tex"), 0);
  glUniform2f(prog->getUniform("resolution"), (float)PostProcessor::w_width, (float)PostProcessor::w_height);
  PostProcessor::drawFSQuad();
  prog->unbind();
}

void applyBrightFilter(GLuint tex, GLuint fbo, Program * prog)
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  
  prog->bind();
  glUniform1i(prog->getUniform("tex"), 0);
  PostProcessor::drawFSQuad();
  prog->unbind();
}

static void applyBloomCombine(GLuint tex, GLuint brightTex, GLuint fbo, Program* combineProg)
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, brightTex);

  combineProg->bind();
  glUniform1i(combineProg->getUniform("tex"), 0);
  glUniform1i(combineProg->getUniform("brightTexture"), 1);
  PostProcessor::drawFSQuad();
  combineProg->unbind();
}