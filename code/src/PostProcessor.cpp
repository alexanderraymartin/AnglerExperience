#include "PostProcessor.h"

static void initQuad(GLuint &quadVAO, GLuint &quadVBO);

static void createFBO(int w_width, int w_height, GLuint& fb, GLuint& tex, GLenum filter, GLenum wrap);

static void applyDepthOfFieldShader(GLuint tex, GLuint fboID, Program* prog, float focusDepth);
static void applyBloomShader(GLuint tex, GLuint fboID, Program* prog);
static void applyCombine(GLuint tex, GLuint brightTex, GLuint fboID, Program* combineProg);


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
  lastout = processBloom(texture);
  lastout = processDepthOfField(texBuf[lastout], 0.5);
  lastout = runFXAA(texBuf[lastout], output);
}

int PostProcessor::processBloom(GLuint texture)
{
  int fboID1 = nextFBO();
  int fboID2 = nextFBO();

  // Bright filter
  applyBloomShader(texture, fboID1, shaderlib->getPtr("bloom_brightFilter"));

  for (int i = 0; i < BLOOM_BLUR_AMOUNT; i++)
  {
    // Horizontal blur
	applyBloomShader(texBuf[fboID1], fboID2, shaderlib->getPtr("bloom_horizontalBlur"));
    
    // Vertical blur
	applyBloomShader(texBuf[fboID2], fboID1, shaderlib->getPtr("bloom_verticalBlur"));
  }

  // Combine
  applyCombine(texture, texBuf[fboID1], fboID2, shaderlib->getPtr("bloom_combine"));
  ASSERT_NO_GLERR();
  return fboID2;
}

int PostProcessor::runFXAA(GLuint texture, int output)
{
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

int PostProcessor::processDepthOfField(GLuint texture, float focusDepth)
{
  int fboID1 = nextFBO();
  int fboID2 = nextFBO();
  
  // Horizontal blur
  applyDepthOfFieldShader(texture, fboID1, shaderlib->getPtr("depthOfField_horizontalBlur"), focusDepth);
      
  // Vertical blur
  applyDepthOfFieldShader(texBuf[fboID1], fboID2, shaderlib->getPtr("depthOfField_verticalBlur"), focusDepth);

  drawFSQuad();
  ASSERT_NO_GLERR();
  return fboID2;
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
static void applyDepthOfFieldShader(GLuint tex, GLuint fboID, Program* prog, float focusDepth)
{
	glBindFramebuffer(GL_FRAMEBUFFER, PostProcessor::frameBuf[fboID]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	prog->bind();
	glUniform1i(prog->getUniform("depthBufTex"), PostProcessor::depthBuf);
	glUniform1i(prog->getUniform("tex"), 0);
	glUniform1f(prog->getUniform("focusDepth"), focusDepth);
	PostProcessor::drawFSQuad();
	prog->unbind();
}

/////////////////////////////////////////////
/* Bloom */
static void applyBloomShader(GLuint tex, GLuint fboID, Program* prog)
{
	glBindFramebuffer(GL_FRAMEBUFFER, PostProcessor::frameBuf[fboID]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	prog->bind();
	glUniform1i(prog->getUniform("tex"), 0);
	PostProcessor::drawFSQuad();
	prog->unbind();
}

static void applyCombine(GLuint tex, GLuint brightTex, GLuint fboID, Program* combineProg)
{
  glBindFramebuffer(GL_FRAMEBUFFER, fboID < 0 ? 0 : PostProcessor::frameBuf[fboID]);
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