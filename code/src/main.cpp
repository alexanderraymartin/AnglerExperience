#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <limits>
#define _USE_MATH_DEFINES
#include <cmath>

#define GLEW_STATIC
#include <GL/glew.h>
#include "utility/GLSL.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <json.hpp>

#include <common.h>
#include "core.h"
#include "GameState.hpp"

// This isn't really necessary as we will be importing implementations of these interfaces
// but for now it's nice to make sure things compile with them there. 
#include "Component.hpp"
#include "Entity.hpp"

#include "components/Geometry.hpp"
#include "SimpleComponents.hpp"
#include "AnimationComponents.hpp"
#include "LightingComponents.hpp"
#include "Material.hpp"
#include "Camera.h"

#include "RenderSystem.hpp"
#include "AnimationSystem.hpp"
#include "MouseProcessing.hpp"
#include "PostProcessor.h"

using namespace std;

// #define FORCEWINDOW

static double mouseX = 0;
static double mouseY = 0;
static Pose* mousePose;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Forward Declarations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static void initGL();
static void initGLFW(ApplicationState &appstate);
static void initShaders(ApplicationState &appstate);
static void initScene(ApplicationState &appstate, GameState &gstate, Camera* camera);

static GLFWmonitor* autoDetectScreen(UINT* width, UINT* height);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void cursor_callback(GLFWwindow *window, double posX, double posY);

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
  initGL();
  initShaders(appstate);
  DynamicCamera* camera = new DynamicCamera();
  initScene(appstate, gstate, camera);

  RenderSystem::init(appstate);

  gstate.gameTime.reset();
  while(!glfwWindowShouldClose(appstate.window)){

    double dt = gstate.fxAnimTime.elapsed();
    // TODO: Appropriate timestep loop structure
    {
      // TODO: PlayerSystem::update(appstate, gstate, elapsedTime);
      // TODO: CameraSystem::update(appstate, gstate, elapsedTime);
		camera->update(appstate.window, dt);
      // TODO: SwarmSystem::update(appstate, gstate, elapsedTime);
      // TODO: PhysicsSystem::update(appstate, gstate, elapsedTime);
      // TODO: ParticleSystem::update(appstate, gstate, elapsedTime); // Particle System System*
      // TODO: GameplaySystem::update(appstate, gstate, elapsedTime);
      AnimationSystem::update(appstate, gstate, dt);
    }

    // Rendering happens here. This 'RenderSystem' will end up containing a lot and effectively
    // utilize many self-contained subsystems (semantically not real systems) which do individual
    // parts of the drawing and operate on different buffers and geometries. However I'd like to
    // try and keep all that linked together inside of the single RenderSystem for simplicity and
    // so that not buffers or other data has to be shared between calls here in main(). 

    mousePose->loc = MouseProcessing::getWoldSpace(mouseX, mouseY, appstate.window, camera);

    RenderSystem::render(appstate, gstate, dt);

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
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glClearColor(0.0, 0.0, 0.0, 1.0);
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

#ifdef FORCEWINDOW
  monitor = NULL;
#endif

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

  glfwGetCursorPos(appstate.window, &mouseX, &mouseY);

  glfwSwapInterval(1);
  // glfwSetInputMode(appstate.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetFramebufferSizeCallback(appstate.window, RenderSystem::onResize);
  glfwSetKeyCallback(appstate.window, key_callback);
  glfwSetCursorPosCallback(appstate.window, cursor_callback);
}

static void initShaders(ApplicationState &appstate){
  appstate.shaderlib.init();

  ifstream shaderfile = ifstream("" STRIFY(SHADER_DIR) "/shaders.json");
  if(!shaderfile.is_open()){
    fprintf(stderr, "Failed to open shaders json file!\n");
    exit(3);
  }
  json shaderjson;
  shaderfile >> shaderjson;
  for(json::iterator it = shaderjson["pairs"].begin(); it != shaderjson["pairs"].end(); it++){
    appstate.shaderlib.add(it.key(), new Program(it.value()));
    cout << "Loaded shader: " << it.key() << endl;
  }
}

static void initScene(ApplicationState &appstate, GameState &gstate, Camera* camera){
  //StaticCamera* scenecam = new StaticCamera(37.5, glm::vec3(0.0, 3.4, 0.0), glm::vec3(0.0, 3.1, 3.0));
	gstate.activeScene = new Scene(camera);

  Entity* sun;
  {
    sun = new Entity();
    sun->attach(new SunLight(
      glm::vec3(.7, .7, .65),
      glm::vec3(0.5, -1.0, 1.0))
    );
  }
  gstate.activeScene->addEntity(sun);

  Entity* pointlight;
  {
    pointlight = new Entity();
    pointlight->attach(new PointLight(glm::vec3(.2), 1.0, 15.0));
    pointlight->attach(new Pose(glm::vec3(-1.5, 0.5, 1.0)));
  }
  gstate.activeScene->addEntity(pointlight);

  Entity* groundplane;
  {
    groundplane = new Entity();

    Material mat("" STRIFY(ASSET_DIR) "/simple-phong.mat");

    vector<Geometry> cubegeo;
    Geometry::loadFullObj( "" STRIFY(ASSET_DIR) "/cube.obj", cubegeo);

    SolidMesh* mesh = new SolidMesh(cubegeo);
    mesh->setMaterial(mat);

    Pose* pose = new Pose(glm::vec3(0.0906, -0.31318, 48.0));
    pose->orient = glm::angleAxis(glm::radians(4.92f), glm::vec3(.793, -.051, .607));
    pose->scale = glm::vec3(45.0, .05, 48.0);
    groundplane->attach(mesh);
    groundplane->attach(pose);
  }

  Entity* minnow;
  {
    minnow = new Entity();

    Material mat("" STRIFY(ASSET_DIR) "/simple-phong.mat");

    vector<SolidMesh*> meshes;
    for (int i = 0; i < 19; i++) {
      vector<Geometry> minnowgeo;
      string num = i < 9 ? string("0") + to_string(i+1) : to_string(i+1);
      Geometry::loadFullObj((string("" STRIFY(ASSET_DIR) "/minnow2/Minnow_0000")
        + num + string(".obj")).c_str(), minnowgeo);
      SolidMesh* mesh = new SolidMesh(minnowgeo);
      mesh->setMaterial(mat);
      meshes.push_back(mesh);
    }

    Pose* pose = new Pose(glm::vec3(0, 3, 5));
    pose->scale = glm::vec3(1, 1, 1);
    pose->orient = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 1, 0));
    minnow->attach(new AnimatableMesh(new Animation(meshes, 0.066)));
    minnow->attach(pose);
  }

  Entity* cube;
  {
    cube = new Entity();

    Material mat("" STRIFY(ASSET_DIR) "/simple-phong.mat");

    vector<Geometry> cubegeo;
    Geometry::loadFullObj( "" STRIFY(ASSET_DIR) "/cube.obj", cubegeo);

    SolidMesh* mesh = new SolidMesh(cubegeo);
    mesh->setMaterial(mat);

    mousePose = new Pose(glm::vec3(0, 3, 10));
    mousePose->scale = glm::vec3(0.1, 0.1, 0.1);
    mousePose->orient = glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0));
    cube->attach(mesh);
    cube->attach(mousePose);
  }

  Entity* cube2;
  {
    cube2 = new Entity();
    
    Material mat("" STRIFY(ASSET_DIR) "/simple-phong.mat");
    
    vector<Geometry> cubegeo;
    Geometry::loadFullObj("" STRIFY(ASSET_DIR) "/cube.obj", cubegeo);
    
    SolidMesh* mesh = new SolidMesh(cubegeo);
    mesh->setMaterial(mat);
    
    Pose* pose = new Pose(glm::vec3(0, 3, 2));
    pose->scale = glm::vec3(0.1, 0.1, 0.1);
    pose->orient = glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0));
    cube2->attach(mesh);
    cube2->attach(pose);
  }

  Entity* cube3;
  {
    cube3 = new Entity();
    
    Material mat("" STRIFY(ASSET_DIR) "/simple-phong.mat");
    
    vector<Geometry> cubegeo;
    Geometry::loadFullObj("" STRIFY(ASSET_DIR) "/cube.obj", cubegeo);
    
    SolidMesh* mesh = new SolidMesh(cubegeo);
    mesh->setMaterial(mat);
    
    Pose* pose = new Pose(glm::vec3(2, 3, 30));
    pose->scale = glm::vec3(0.1, 0.1, 0.1);
    pose->orient = glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0));
    cube3->attach(mesh);
    cube3->attach(pose);
  }

  gstate.activeScene->addEntity(minnow);

  gstate.activeScene->addEntity(groundplane);
  gstate.activeScene->addEntity(cube);
  gstate.activeScene->addEntity(cube2);
  gstate.activeScene->addEntity(cube3);

  gstate.activeScene->addEntity(sun);
  gstate.activeScene->addEntity(pointlight);
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

static void cursor_callback(GLFWwindow *window, double posX, double posY)
{
  mouseX = posX;
  mouseY = posY;
}

// This should be considered temporary untill a proper menu is established. In playable
// builds ESC should pause not quit. 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}
