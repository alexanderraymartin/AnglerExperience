#pragma once
#ifndef POSTPROCESSOR_H_
#define POSTPROCESSOR_H_

#include <iostream>
#include <fstream>
#include "GLSL.h"
#include "Program.h"
#include "utility/ShaderLibrary.hpp"
#include <GLFW/glfw3.h>
#include <common.h>

#define ABS(_X) (_X >= 0 ? _X : -_X)

#define POSTPROCESSOR_BUFFER_COUNT 2
#define POSTPROCESSOR_LOW_RES_BUFFER_COUNT 2

#define BLOOM_BLUR_AMOUNT 3
#define DEPTH_OF_FIELD_BLUR_AMOUNT 3
#define LOW_RES_FBO_SCALE 4

#define HAS_POST_PROCESSING true

namespace PostProcessor{

  static int w_width;
  static int w_height;

  static GLuint quadVAO;
  static GLuint quadVBO;

  // full screen buffers
  static GLuint frameBuf[POSTPROCESSOR_BUFFER_COUNT];
  static GLuint texBuf[POSTPROCESSOR_BUFFER_COUNT];

  // lower resolution buffers
  static GLuint lowResFrameBuf[POSTPROCESSOR_LOW_RES_BUFFER_COUNT];
  static GLuint lowResTexBuf[POSTPROCESSOR_LOW_RES_BUFFER_COUNT];

  static UINT _nextFBO = 0;
  static UINT _nextLowResFBO = 0;

  static ShaderLibrary* shaderlib = nullptr;

  void init(int w_width, int w_height, ShaderLibrary* shaderlib);

  void doPostProcessing(GLuint texture, GLuint depthBuffer);
  void passThroughShader(GLuint texture);
  int processBloom(GLuint texture, bool isLast);
  int runFXAA(GLuint texture, bool isLast);
  int processDepthOfField(GLuint texture, GLuint depthBuffer, bool isLast);
  void drawFSQuad();

  static UINT nextLowResFBO() { return((_nextLowResFBO = ++_nextLowResFBO % POSTPROCESSOR_LOW_RES_BUFFER_COUNT)); }
  static UINT lastLowResFBO() { return(ABS(_nextLowResFBO - 1) % POSTPROCESSOR_LOW_RES_BUFFER_COUNT); }
  static UINT offsetLowResFBO(int offset) { return(ABS(_nextLowResFBO + offset) % POSTPROCESSOR_LOW_RES_BUFFER_COUNT); }

  static UINT nextFBO() {return((_nextFBO = ++_nextFBO % POSTPROCESSOR_BUFFER_COUNT));}
  static UINT lastFBO() {return(ABS(_nextFBO - 1) % POSTPROCESSOR_BUFFER_COUNT);}
  static UINT offsetFBO(int offset) {return(ABS(_nextFBO + offset) % POSTPROCESSOR_BUFFER_COUNT);}

  static void resize(int w_width, int w_height) {init(w_width, w_height, shaderlib);};
}

#endif