#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <cmath>

#define GLEW_STATIC
#include <GL/glew.h>
#include "utility/GLSL.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <common.h>
#include "core.h"
#include "GameState.hpp"

/* to use glee */
#define GLEE_OVERWRITE_GL_FUNCTIONS
#include "utility/glee.h"

// This isn't really necessary as we will be importing implementations of these interfaces
// but for now it's nice to make sure things compile with them there. 
#include "Component.hpp"
#include "Entity.hpp"

#include "components/Geometry.hpp"
#include "SimpleComponents.hpp"
#include "AnimationComponents.hpp"

#include "RenderSystem.hpp"
#include "AnimationSystem.hpp"

using namespace std;


// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Forward Declarations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static void initGL();
static void initLibs(TopLevelResources &resources);
static void initGLFW(ApplicationState &appstate);
static void initShaders(ApplicationState &appstate);
static void initPrimitives(TopLevelResources &resources);
static void initScene(ApplicationState &appstate, GameState &gstate);

static GLFWmonitor* autoDetectScreen(UINT* width, UINT* height);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // END Forward Declarations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=





// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Main()
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

int main(int argc, char** argv){
  ApplicationState appstate; 
  GameState gstate(glfwGetTime); // glfwGetTime used as source of time. It can be replaced.

  srand(time(NULL));
  initGLFW(appstate);
  initLibs(appstate.resources); // Can be split up in needed
  initGL();
  initShaders(appstate);
  initPrimitives(appstate.resources);

  initScene(appstate, gstate);

  RenderSystem::init(appstate);

  gstate.gameTime.reset();
  while(!glfwWindowShouldClose(appstate.window)){

    double fxAnimTime = gstate.fxAnimTime.elapsed();
    // TODO: Appropriate timestep loop structure
    {
      // TODO: PlayerSystem::update(appstate, gstate, elapsedTime);
      // TODO: CameraSystem::update(appstate, gstate, elapsedTime);
      // TODO: SwarmSystem::update(appstate, gstate, elapsedTime);
      // TODO: PhysicsSystem::update(appstate, gstate, elapsedTime);
      // TODO: ParticleSystem::update(appstate, gstate, elapsedTime); // Particle System System*
      // TODO: GameplaySystem::update(appstate, gstate, elapsedTime);
      AnimationSystem::update(appstate, gstate, fxAnimTime);
    }

    // Rendering happens here. This 'RenderSystem' will end up containing a lot and effectively
    // utilize many self-contained subsystems (semantically not real systems) which do individual
    // parts of the drawing and operate on different buffers and geometries. However I'd like to
    // try and keep all that linked together inside of the single RenderSystem for simplicity and
    // so that not buffers or other data has to be shared between calls here in main(). 

    RenderSystem::render(appstate, gstate, fxAnimTime);

    glfwSwapBuffers(appstate.window);
    glfwPollEvents();

    appstate.framesCompleted++;
    appstate.firstRun = false;
  }

  printf("Exiting...\nCompleted a total of %lu frames in lifetime of application\n", appstate.framesCompleted);

  // TODO: Add any other SDK or library cleanup here. 
  glfwDestroyWindow(appstate.window);
  glfwTerminate();

  return(0);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // END Main()
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=





// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Initialization Functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static void initGL(){
  glewExperimental = true;
  if(glewInit() != GLEW_OK) {
    cerr << "Failed to initialize GLEW" << endl;
    exit(-1);
  }
  glGetError();
  cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
  cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

  GLSL::checkVersion();
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0, 0.0, 0.0, 1.0);
}

static void initLibs(TopLevelResources &resources){
  // SFML
  // IMGUI
  // ASSIMP
  // ...?
}

// This is verbose and ugly. Maybe we should move it.
static void initGLFW(ApplicationState &appstate){
  // Lambda functions for simple callbacks
  auto error_callback = [](int error, const char* description){cerr << description << endl;};

  glfwSetErrorCallback(error_callback);

  if(!glfwInit()) {
    exit(-1);
  }
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  // Later this should only run if settings are not already loaded from a save/config
  UINT w_width = 854;
  UINT w_height = 480;

  GLFWmonitor* monitor = autoDetectScreen(&w_width, &w_height);

  fprintf(stderr, "Auto-selected %ux%u %s for screen config\n",
    w_width,
    w_height,
    monitor == NULL ? "windowed" : "fullscreen"
  );

  appstate.window = glfwCreateWindow(w_width, w_height, "Angler Experience!", monitor, NULL);
  
  if(!appstate.window) {
    glfwTerminate();
    exit(-1);
  }
  glfwMakeContextCurrent(appstate.window);

  glfwSwapInterval(1);
  // glfwSetInputMode(appstate.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetFramebufferSizeCallback(appstate.window, RenderSystem::onResize);
  glfwSetKeyCallback(appstate.window, key_callback);
}

static void initShaders(ApplicationState &appstate){
  appstate.resources.shaderlib.init();

  // TODO: Iterate through given shader source files, compile them, and store the in the shaderlib.
  // Note: To prevent this from being ungodly long due to the nature of Zoe's Program class we should 
  // probably set up some kind of alternative for adding all the uniforms and attributes. JSON loader? 
}
static void initPrimitives(TopLevelResources &resources){
  // TODO: Load some primitive geometry into appropriate OpenGL buffers. (quads, tris, cube, sphere, ect...)
}

static void initScene(ApplicationState &appstate, GameState &gstate){
  StaticCamera* scenecam = new StaticCamera(45.0, glm::vec3(0.0), glm::vec3(0.0, 0.0, 3.0));
  gstate.activeScene = new Scene(scenecam);

  Entity *cube = new Entity();
  
  vector<Geometry> cubegeo;
  Geometry::loadFullObj("../gameassets/cube.obj", cubegeo);

  cube->attach(new SolidMesh(cubegeo));
  cube->attach(new Pose(glm::vec3(0.0, 0.0, 6.0)));
  cube->attach(new LinearRotationAnim(glm::vec3(0.0,1.0,0.0), 10.0/(2.0*M_PI)));
  gstate.activeScene->addEntity(cube);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // END Initialization Functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=





// A rudimentary helper function for automatically choosing appropriate windowing on startup. 
// This should eventually be replaced with a more sophisticated utility and/or only be used
// if no settings configuration has already been saved. 
static GLFWmonitor* autoDetectScreen(UINT* width, UINT* height){
  static const UINT pairs16by9[6][2] = {
    {3840, 4320},
    {2560, 1440},
    {1920, 1080},
    {1280, 720},
    {853, 480},
    {640, 360}
  };

  GLFWmonitor* primary = glfwGetPrimaryMonitor();
  const GLFWvidmode* pmodes = glfwGetVideoMode(primary);

  *width = pmodes->width;
  *height = pmodes->height;

  // Check if aspect ratio is appropriate. If not put in a window. Control over this can be handed to user later.
  if(fabs(static_cast<double>(pmodes->width)/static_cast<double>(pmodes->height) - 16.0/9.0) >= .19)
  {
    //Probably not widescreen, put them in a window
    int i;
    for(i = 0; i < 6; i++)
    {
      if(*width > pairs16by9[i][0] && *height > pairs16by9[i][1])
      {
        break;
      }
    }

    *width = pairs16by9[i][0];
    *height = pairs16by9[i][1];
    return(NULL);
  }
  else
  {
    // Either 16:9 or 16:10, should be fine in fullscreen. 
    return(primary);
  }
}

// This should be considered temporary untill a proper menu is established. In playable
// builds ESC should pause not quit. 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}
