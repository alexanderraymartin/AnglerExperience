#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <limits>
#define _USE_MATH_DEFINES
#include <cmath>
#include <random>

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
#include "Material.hpp"
#include "Camera.h"
#include "Spawner.h"

#include "RenderSystem.hpp"
#include "AnimationSystem.hpp"
#include "GameplaySystem.hpp"
#include "MouseProcessing.hpp"
#include "AntennaGenerator.hpp"
#include "PostProcessor.h"
#include "SwarmSystem.hpp"
#include "GameplaySystem.hpp"
#include "utility/2dText/text2D.hpp"

using namespace std;

// #define FORCEWINDOW
#define FORCE_WIDTH 1280
#define FORCE_HEIGHT 720

static SolidMesh* antennaMesh;
static vector<Spawner*> spawners;
static default_random_engine generator;
static uniform_real_distribution<float> distribution(0,1);
static PointLight* anglerLight;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Forward Declarations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

static void initGL();
static void initGLFW(ApplicationState &appstate);
static void initShaders(ApplicationState &appstate);
static void initLocations(float ***fishLoc, float ***headLoc, int num_fish);
static void initScene(ApplicationState &appstate, GameState &gstate, Camera* camera);

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
  float **fishLoc = NULL, **headLoc = NULL;

  srand(time(NULL));
  initGLFW(appstate);
  initGL();
  initShaders(appstate);
  DynamicCamera* camera = new DynamicCamera();
  initLocations(&fishLoc, &headLoc, 50);
  initScene(appstate, gstate, camera);

  RenderSystem::init(appstate);
  SwarmSystem::init(appstate, gstate, fishLoc);

  double mouseX, mouseY;
  AntennaGenerator *antennaGen = new AntennaGenerator();

  initText2D("" STRIFY(ASSET_DIR) "/Holstein.DDS", "" STRIFY(SHADER_DIR) "/TextVertexShader.vertexshader", "" STRIFY(SHADER_DIR) "/TextVertexShader.fragmentshader");
  int fishEaten = 0;

  gstate.gameTime.reset();
  while(!glfwWindowShouldClose(appstate.window)){


    glfwGetCursorPos(appstate.window, &mouseX, &mouseY);
    vec3 mousePos = MouseProcessing::getWoldSpace(mouseX, mouseY, appstate.window, camera);

    double fxdt = gstate.fxAnimTime.elapsed();
    double gdt = gstate.gameTime.elapsed();

	  camera->update(appstate.window, fxdt);
    for (Spawner* s : spawners) {
      s->update(gstate, fxdt);
    }
      
    SwarmSystem::update(appstate, gstate, fxdt, &fishLoc, &headLoc, 2.5, mousePos.x, mousePos.y);
    GameplaySystem::update(appstate, gstate, gdt, &fishLoc, &headLoc);
    // AnimationSystem::update(appstate, gstate, fxdt);

    //An FPS Counter so we can see how things are going
    cout << "FPS: " << round(1.0 / fxdt) << "\n";

    Geometry* geo = antennaGen->generateAntenna(vec3(-0.87f, 3.29f, 3.1f), mousePos);
    antennaMesh->geometries = {*geo};
	//not working, not sure why
	anglerLight->setPosition(mousePos);

    RenderSystem::render(appstate, gstate, fxdt);

    glClear(GL_DEPTH_BUFFER_BIT);
    char text[256];
    sprintf(text,"%d fish", fishEaten );
    printText2D(text, 10, 550, 30);

    glfwSwapBuffers(appstate.window);
    glfwPollEvents();

    fishEaten += gstate.activeScene->killqueue.size() / 2;
	
	if (gstate.activeScene->killqueue.size() / 2 > 0) {
		camera->shake(2, 1);
	}

    gstate.activeScene->executeKillQueue();

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
  fprintf(stderr, "Forced Window\n");
  monitor = nullptr;
#if defined FORCE_WIDTH && defined FORCE_HEIGHT
  w_width = FORCE_WIDTH;
  w_height = FORCE_HEIGHT;
#endif 
#endif


  fprintf(stderr, "Auto-selected %ux%u %s for screen config\n",
    w_width,
    w_height,
    monitor == nullptr ? "windowed" : "fullscreen"
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

  // Silence deferred export so tex coords don't complain 
  Program* defexprt = nullptr;
  if ((defexprt = appstate.shaderlib.getPtr("deferred-export"))) {
	  defexprt->setVerbose(false);
  }
}

static void initLocations(float ***fishLoc, float ***headLoc, int num_fish) {
    *fishLoc = (float **) malloc((num_fish) * sizeof(float *));
    *headLoc = (float **) malloc((num_fish) * sizeof(float *));
    for (int i = 0; i < num_fish; i++) {
       (*fishLoc)[i] = (float *) malloc((2) * sizeof(float));
       (*headLoc)[i] = (float *) malloc((2) * sizeof(float));
    }
    srand(time(NULL));
    for (int i = 0; i < num_fish; i++) {
        (*fishLoc)[i][0] = -0 + (((float)rand()/(float)(RAND_MAX)) * 1.0); 
        (*fishLoc)[i][1] = 2.5 + (((float)rand()/(float)(RAND_MAX)) * 1.0); 
        (*headLoc)[i][0] = 0 + (((float)rand()/(float)(RAND_MAX)) * 1.0); 
        (*headLoc)[i][1] = 2.5 + (((float)rand()/(float)(RAND_MAX)) * 1.0); 
    } 
}

static Entity* cubeTemplate = nullptr;
static Entity* createCube(vec3 location) {
  location.x += (rand() % 100) / 10.0f - 5;
  if (!cubeTemplate) {
    vector<Geometry> cubegeo;
    //Try to do load calls only once, not every time create is called. May require more work to do this effictively
    Geometry::loadFullObj("" STRIFY(ASSET_DIR) "/cube.obj", cubegeo);
    cubeTemplate = new Entity();
    Material mat("" STRIFY(ASSET_DIR) "/simple-phong.mat");
    SolidMesh* mesh = new SolidMesh(cubegeo);
    mesh->setMaterial(mat);
    Pose* pose = new Pose(location);
    pose->scale = glm::vec3(0.1, 0.1, 0.1);
    pose->orient = glm::angleAxis(glm::radians(45.0f), glm::vec3(0, 1, 0));
    cubeTemplate->attach(mesh);
    cubeTemplate->attach(pose);
  }

  Entity* newcube = cubeTemplate->clone();
  Pose* pose = cubeTemplate->getFirstComponentType<Pose>();
  Pose* oldpose = cubeTemplate->getFirstComponentType<Pose>();
  if (oldpose && pose) {
    *pose = *oldpose;
    pose->loc = location;
  }
  return(newcube);

}

static Entity* createPointLightEntity(vec3 location) {
	Entity* light = new Entity();
	vec3 color = vec3(distribution(generator), distribution(generator), distribution(generator));
	location += vec3(distribution(generator), 0, distribution(generator)) * vec3(10.0f) - vec3(5.0f);
	PointLight* pointLight = new PointLight(location, color);
	light->attach(pointLight);

	return light;
}

static void initScene(ApplicationState &appstate, GameState &gstate, Camera* camera){
  //StaticCamera* scenecam = new StaticCamera(37.5, glm::vec3(0.0, 3.4, 0.0), glm::vec3(0.0, 3.1, 3.0));
	gstate.activeScene = new Scene(camera);

  Entity* sun;
  {
    sun = new Entity();
    sun->attach(new SunLight(
      glm::vec3(.2, .2, .19),
      glm::vec3(0.5, -1.0, 1.0),
      glm::vec3(0.0f, 10.0f, 0.0f)
    ));
  }
  gstate.activeScene->addEntity(sun);


  Entity* groundplane;
  {
    groundplane = new Entity();

    Material mat("" STRIFY(ASSET_DIR) "/seafloor.mat");

    vector<Geometry> cubegeo;
    Geometry::loadFullObj( "" STRIFY(ASSET_DIR) "/LandscapeBasis.obj", cubegeo);

    SolidMesh* mesh = new SolidMesh(cubegeo);
    mesh->setMaterial(mat);
    mesh->isSweepable = false;

    groundplane->attach(mesh);
    groundplane->attach(new GroundPlane(gstate.levelOrigin, gstate.levelOrientation, gstate.levelScale));
  }

  vec3 cubeLoc = vec3(0, 3, 10);
  Entity* mamaCube = new Entity();
  {
	  Spawner* s = new Spawner(cubeLoc, &createCube);
	  mamaCube->attach(s);
	  // spawners.push_back(s);
  }

  Entity* lightSpawner = new Entity();
  {
	  Spawner* s = new Spawner(vec3(0,0,0), &createPointLightEntity);
	  lightSpawner->attach(s);
	  spawners.push_back(s);
  }


 
  Entity* angler;
  {
	  angler = new Entity();
	  vector<Geometry> geo;
    Material body("" STRIFY(ASSET_DIR) "/angbody.mat");
    Material teeth("" STRIFY(ASSET_DIR) "/angteeth.mat");
    Material eye("" STRIFY(ASSET_DIR) "/angeye.mat");
	  Geometry::loadFullObj("" STRIFY(ASSET_DIR) "/Angler.obj", geo);
    geo[0].material = eye;
    geo[1].material = teeth;
    geo[2].material = body;

    vec3 loc = vec3(-1.2, 3.12, 3.0);
	  SolidMesh* mesh = new SolidMesh(geo);
	  Pose* pose = new Pose(loc);
	  pose->scale = glm::vec3(.4, .4, .4);
	  pose->orient = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 1, 0));
	  angler->attach(mesh);
	  angler->attach(pose);
    anglerLight = new PointLight(loc, vec3(1.0, 0.5, 0.2));
    angler->attach(anglerLight);
  }gstate.activeScene->addEntity(angler);

  Entity* antenna;
  {
    antenna = new Entity();

    Material mat("" STRIFY(ASSET_DIR) "/simple-phong.mat");

    vector<Geometry> antennaGeo = vector<Geometry>();
    antennaMesh = new SolidMesh(antennaGeo);
    antennaMesh->setMaterial(mat);
    antenna->attach(antennaMesh);
  }


  gstate.activeScene->addEntity(lightSpawner);
  gstate.activeScene->addEntity(groundplane);
  gstate.activeScene->addEntity(antenna);

  gstate.activeScene->addEntity(sun);

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
