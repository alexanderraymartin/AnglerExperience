#include "PostProcessor.h"

static void initQuad(GLuint &quadVAO, GLuint &quadVBO);

static void createFBO(int w_width, int w_height, GLuint& fb, GLuint& tex, GLenum filter, GLenum wrap);

static void applyDepthOfFieldCombine(GLuint tex, GLuint blurredTex, GLuint depthBuffer, GLuint fbo, Program* combineProg);
static void applyShaderToTexture(GLuint tex, GLuint fbo, Program* prog);
static void applyBloomCombine(GLuint tex, GLuint brightTex, GLuint fbo, Program* combineProg);


void PostProcessor::init(int _w_width, int _w_height, ShaderLibrary* _shaderlib)
{
  shaderlib = _shaderlib;
  w_width = _w_width;
  w_height = _w_height;

  initQuad(quadVAO, quadVBO);

  //create three frame buffer objects to toggle between
  glGenFramebuffers(POSTPROCESSOR_BUFFER_COUNT, frameBuf);
  glGenTextures(POSTPROCESSOR_BUFFER_COUNT, texBuf);

  for(int i = 0; i < POSTPROCESSOR_BUFFER_COUNT; i++){
    createFBO(w_width, w_height, frameBuf[i], texBuf[i], GL_LINEAR, GL_CLAMP_TO_EDGE);
  }
}

void PostProcessor::doPostProcessing(GLuint texture, GLuint depthBuffer)
{
  int lastout;
  //lastout = processBloom(texture, false);
  lastout = processDepthOfField(texture, depthBuffer, false);
  //lastout = processDepthOfField(texBuf[lastout], depthBuffer, false);
  lastout = runFXAA(texBuf[lastout], true);
}

int PostProcessor::processBloom(GLuint texture, bool isLast)
{
  int fboID1 = nextFBO();
  // Bloom needs different sized frame buffers
  GLuint bloomFrameBuf[2];
  GLuint bloomTexBuf[2];

  glGenFramebuffers(2, bloomFrameBuf);
  glGenTextures(2, bloomTexBuf);

  createFBO(w_width / BLOOM_FBO_DOWN_SCALE, w_height / BLOOM_FBO_DOWN_SCALE, bloomFrameBuf[0], bloomTexBuf[0], GL_LINEAR, GL_CLAMP_TO_EDGE);
  createFBO(w_width / BLOOM_FBO_DOWN_SCALE, w_height / BLOOM_FBO_DOWN_SCALE, bloomFrameBuf[1], bloomTexBuf[1], GL_LINEAR, GL_CLAMP_TO_EDGE);

  // Bright filter
  glViewport(0, 0, w_width / BLOOM_FBO_DOWN_SCALE, w_height / BLOOM_FBO_DOWN_SCALE);
  applyShaderToTexture(texture, bloomFrameBuf[0], shaderlib->getPtr("bloom_brightFilter"));

  for (int i = 0; i < BLOOM_BLUR_AMOUNT; i++)
  {
    // Horizontal blur
	applyShaderToTexture(bloomTexBuf[0], bloomFrameBuf[1], shaderlib->getPtr("bloom_horizontalBlur"));
    
    // Vertical blur
	applyShaderToTexture(bloomTexBuf[1], bloomFrameBuf[0], shaderlib->getPtr("bloom_verticalBlur"));
  }

  // Combine
  glViewport(0, 0, w_width, w_height);
  applyBloomCombine(texture, bloomTexBuf[0], isLast ? 0 : frameBuf[fboID1], shaderlib->getPtr("bloom_combine"));
  ASSERT_NO_GLERR();
  return fboID1;
}

int PostProcessor::processDepthOfField(GLuint texture, GLuint depthBuffer, bool isLast)
{
	int fboID1 = nextFBO();
	int fboID2 = nextFBO();

	// Horizontal blur 1st pass
	applyShaderToTexture(texture, frameBuf[fboID1], shaderlib->getPtr("depthOfField_horizontalBlur"));

	// Vertical blur 1st pass
	applyShaderToTexture(texBuf[fboID1], frameBuf[fboID2], shaderlib->getPtr("depthOfField_verticalBlur"));

	for (int i = 0; i < DEPTH_OF_FIELD_BLUR_AMOUNT - 1; i++)
	{
		// Horizontal blur mutli-pass
		applyShaderToTexture(texBuf[fboID2], frameBuf[fboID1], shaderlib->getPtr("depthOfField_horizontalBlur"));

		// Vertical blur mutli-pass
		applyShaderToTexture(texBuf[fboID1], frameBuf[fboID2], shaderlib->getPtr("depthOfField_verticalBlur"));
	}
	
	// Combine blur with full res texture
	applyDepthOfFieldCombine(texture, texBuf[fboID2], depthBuffer, isLast ? 0 : frameBuf[fboID1], shaderlib->getPtr("depthOfField_depthOfFieldCombine"));

	ASSERT_NO_GLERR();
	return fboID1;
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
static void applyShaderToTexture(GLuint tex, GLuint fbo, Program* prog)
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