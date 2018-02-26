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


namespace PostProcessor{

  static int w_width;
  static int w_height;

  static GLuint quadVAO;
  static GLuint quadVBO;

  static GLuint frameBuf[2];
  static GLuint texBuf[2];
  static UINT _nextFBO = 0;

  static GLuint depthBuf;

  static bool _hasBloom = true;
  static bool _hasFXAA = true;

  static ShaderLibrary* shaderlib = nullptr;

  void init(int w_width, int w_height, ShaderLibrary* shaderlib);

  void doPostProcessing(GLuint texture, GLuint output);
  void processBloom(GLuint texture, int output);
  void runFXAA(GLuint texture, int output);
  void drawFSQuad();

  static void toggleBloom() {_hasBloom = !_hasBloom;}
  static bool hasBloom() {return(_hasBloom);}
  static void toggleFXAA() {_hasFXAA = !_hasFXAA;}
  static bool hasFXAA() {return(_hasFXAA);}

  static UINT nextFBO(){return((_nextFBO = ++_nextFBO % 2));}
  static void resize(int w_width, int w_height) {init(w_width, w_height, shaderlib);};
  
}

#endif