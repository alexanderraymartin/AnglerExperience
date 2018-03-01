#pragma once
#ifndef TIMELINE_H_
#define TIMELINE_H_

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "common.h"

template <typename T>
class Timeline{
public:
  Timeline(T (*timeSource)());
  Timeline();
  void setSource(T (*timeSource)());
  T getTime();
  T get() {return(getTime());}
  T elapsed();
  T reset();
  void pause();
  void unpause();
  void togglePause();

  T lastTime;
protected:
  T (*timeSource)()=NULL;
  T start;
  T pausetime;
  
  bool paused = false;
};

#endif