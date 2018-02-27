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

namespace PostProcessor{

  static int w_width;
  static int w_height;

  static GLuint quadVAO;
  static GLuint quadVBO;

  static GLuint frameBuf[POSTPROCESSOR_BUFFER_COUNT];
  static GLuint texBuf[POSTPROCESSOR_BUFFER_COUNT];

  static UINT _nextFBO = 0;

  static GLuint depthBuf;

  static ShaderLibrary* shaderlib = nullptr;

  void init(int w_width, int w_height, ShaderLibrary* shaderlib);

  void doPostProcessing(GLuint texture, GLuint output);
  int processBloom(GLuint texture, int output);
  int runFXAA(GLuint texture, int output);
  void drawFSQuad();

  static UINT nextFBO() {return((_nextFBO = ++_nextFBO % POSTPROCESSOR_BUFFER_COUNT));}
  static UINT lastFBO() {return(ABS(_nextFBO - 1) % POSTPROCESSOR_BUFFER_COUNT);}
  static UINT offsetFBO(int offset) {return(ABS(_nextFBO + offset) % POSTPROCESSOR_BUFFER_COUNT);}
  static void resize(int w_width, int w_height) {init(w_width, w_height, shaderlib);};
}

#endif