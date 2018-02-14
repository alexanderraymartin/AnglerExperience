#pragma once
#ifndef CORE_H_
#define CORE_H_

#include "utility/common.h"
#include <stdlib.h>

#include "utility/ShaderLibrary.hpp"
//#include "utility/FontLibrary.hpp"

#include <GLFW/glfw3.h>

#define APPSTATE ApplicationState
#define TLRES TopLevelResources

using namespace std;

// Application type should be set to differentiate between internally between an instance of the 
// game and an instance of a separate tool. 
// The preprocessor constant 'APPTYPE' should be set to one of the types defined in the following 
// enum and should be set at compile time by CMAKE when building the appropriate target. 
// In Cmake the constant can be set with a command like...
// target_compile_definitions(<target> PUBLIC APPTYPE=ApplicationType::GAME )
enum class ApplicationType{
  GAME,
  ASSET_FIDDLE,
  SWARM_FIDDLE,
  COLLISION_TEST
  // Add more as you see fit
};

struct TopLevelResources{
    ShaderLibrary shaderlib;
    // FontLibrary fonts;
    // Library/SDK instances 
};

// We can continue to fill this out as we move forward. It should only hold information key to the
// very core c++ application, and not anything directly tied to gameplay or assets
struct ApplicationState{
  GLFWwindow* window;
  ApplicationType apptype = APPTYPE;

  bool firstRun = true; // In case of edge cases where special ops need to be done on first loop;n

  // Application Settings such as graphics feature on/off

  // Global stats collection such as number of total frames rendered
  unsigned long framesCompleted = 0;

  TopLevelResources resources;
  
};



#endif