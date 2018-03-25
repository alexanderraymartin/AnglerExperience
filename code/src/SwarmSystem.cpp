 #include "SwarmSystem.hpp"
#include <common.h>
#include <Program.h>

#include <iostream>

#include "core.h"
#include "GameState.hpp"
#include "Entity.hpp"

#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "components/Geometry.hpp"
#include "components/SimpleComponents.hpp"
#include "components/AnimationComponents.hpp"

#include "AnimationComponents.hpp"

#define SPEED .005f
#define SWARM_RADIUS .1f
#define FISH_RADIUS .05f
#define NUM_FISH 50
#define DEPTH 1.75
#define BAIT_RADIUS .1f
#define AFFECTOR_THRESH 1.5f


// Disable complaints about all the placeholder functions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Forward Declarations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// draw Functions start general and get more specific

static void updateSwarm(Entity* entity, double dt, float ***fishLoc, float ***headLoc, GLFWwindow* window, float groundH, float mx, float my);

static vec3 getAverageHeading(int fish, int numFish, float **fishLoc, float **headLoc);

static vec3 getAveragePosition(int fish, int numFish, float **fishLoc, float **headLoc, float groundH);

static vec3 avoidCollision(int fish, int numFish, vec3 velocity, float **fishLoc, float **headLoc);

static void flock(int fish, int numFish, float ***fishLoc, float ***headLoc, float dt, float mx, float my, float groundH);

static float getCursorCoord(float cord, float depth, int dimen);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // SwarmSystem functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void SwarmSystem::update(ApplicationState &appstate, GameState &gstate, double elapsedTime, float ***fishLoc, float ***headLoc, float groundH, float mx, float my){
  for(pair<const Entity*, Entity*> entpair : gstate.activeScene->entities){
    updateSwarm(entpair.second, elapsedTime, fishLoc, headLoc, appstate.window, groundH, mx, my);
  }
}

void SwarmSystem::init(ApplicationState &appstate, GameState &gstate, float **fishLoc) {
    Material mat("" STRIFY(ASSET_DIR) "/simple-phong.mat");
    vector<SolidMesh*> meshes;
    for (int i = 0; i < 19; i++) {
      vector<Geometry> minnowgeo;
      string num = i < 9 ? string("0") + to_string(i+1) : to_string(i+1);
      Geometry::loadFullObj((string("" STRIFY(ASSET_DIR) "/minnow_fix/Minnow_0000")
        + num + string(".obj")).c_str(), minnowgeo);
      SolidMesh* mesh = new SolidMesh(minnowgeo);
      mesh->setMaterial(mat);
      meshes.push_back(mesh);
    }

    Entity* fish;
    {
    for (int i = 0; i < NUM_FISH; i++) {
      fish = new Entity();
      Swarmable* swarm = new Swarmable();
      swarm->fishNum = i;
      Pose* pose = new Pose(glm::vec3(fishLoc[i][0], fishLoc[i][1], 1.0));
      pose->scale = glm::vec3(.025, .025, .025);
      pose->orient = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0, 1, 0));
      fish->attach(pose);
      fish->attach(swarm);
      fish->attach(new AnimatableMesh(new Animation(meshes, 0.066)));
      gstate.activeScene->addEntity(fish);
  }
  }


}

static void updateSwarm(Entity* entity, double dt, float ***fishLoc, float ***headLoc, GLFWwindow* window, float groundH, float mx, float my){
  Pose* pose = NULL;
  Swarmable* swarm = NULL;
  double xpos, ypos;
  int height, width;

  glfwGetFramebufferSize(window, &width, &height);
  glfwGetCursorPos(window, &xpos, &ypos);
  //cout << "xpos " << xpos <<  " ypos " << ypos << endl;

  xpos = -getCursorCoord(xpos, DEPTH, width / 2);
  ypos = -getCursorCoord(ypos, DEPTH, height / 2);  

  //vector<SwarmComponent*> swarmcomps;
  for(Component *cmpnt : entity->components){
    GATHER_SINGLE_COMPONENT(pose, Pose*, cmpnt);
    GATHER_SINGLE_COMPONENT(swarm, Swarmable*, cmpnt);
    if(pose && swarm){
      flock(swarm->fishNum, NUM_FISH, fishLoc, headLoc, dt, mx, my , groundH);
      pose->loc = vec3((*fishLoc)[swarm->fishNum][0], (*fishLoc)[swarm->fishNum][1], DEPTH);
    }
  }
}

static vec3 getAverageHeading(int fish, int numFish, float **fishLoc, float **headLoc) {
  float count = 0;
  vec3 averageHeading = vec3(0, 0, 0);

  for(int i = 0; i < numFish; i++) {
    if (distance(vec3(fishLoc[fish][0], fishLoc[fish][1], 0), vec3(fishLoc[i][0], fishLoc[i][1], 0)) < SWARM_RADIUS) {
      averageHeading = (averageHeading + vec3(headLoc[i][0], headLoc[i][1], 0));
      count++;
    }
  }
  return (vec3) (averageHeading / (float) count);
}


static vec3 getAveragePosition(int fish, int numFish, float **fishLoc, float **headLoc) {
  float count = 0;

  vec3 averagePosition = vec3(0,0,0);
  for (int i = 0; i < numFish; i++) {
    if (distance(vec3(fishLoc[fish][0], fishLoc[fish][1], 0), vec3(fishLoc[i][0], fishLoc[i][1], 0)) < SWARM_RADIUS) {
      averagePosition = (averagePosition + vec3(fishLoc[i][0], fishLoc[i][1], 0));
      count++;
    }
  }
  return (vec3) (averagePosition / (float) count);

}


static vec3 avoidCollision(int fish, int numFish, vec3 velocity, float **fishLoc, float **headLoc, float groundH) {
  float count = 0;
  float dist = 0;
  vec3 averagePosition = vec3(0,0,0);
  vec3 newVelocity = vec3(0,0,0);
  vec3 retVelocity = vec3(0,0,0);

  for (int j = 0; j < numFish; j++) {
    if (fish != j) {
      if (distance(vec3(fishLoc[fish][0], fishLoc[fish][1], 0), vec3(fishLoc[j][0], fishLoc[j][1], 0)) < FISH_RADIUS) { 
        averagePosition += vec3(fishLoc[j][0], fishLoc[j][1], 0);
        count++;
      }
    }
  }
  if (count > 0) {
    averagePosition = (vec3) (averagePosition / count);
    dist = distance(vec3(fishLoc[fish][0], fishLoc[fish][1], 0), averagePosition);
    newVelocity.x = ((SPEED / dist) * (averagePosition.x - fishLoc[fish][0]) * -1);
    newVelocity.y = ((SPEED / dist) * (averagePosition.y - fishLoc[fish][1]) * -1);
    retVelocity = newVelocity;
  }
  else {
    retVelocity = velocity;
  }
  if (fishLoc[fish][1] < groundH) {
    fishLoc[fish][1] += .01; 
  } 
  return retVelocity;   
}

static void flock(int fish, int numFish, float ***fishLoc, float ***headLoc, float dt, float mx, float my, float groundH) {
  float dist = 0;
  vec3 velocity = vec3(0,0,0);
  vec3 averagePosition = vec3(0,0,0);
  vec3 averageDirection = vec3(0,0,0);
  vec3 averageHeading = vec3(0,0,0);
  //mx *= 3;
  //my = (my - 2) * 3;
  for (int i = 0; i < numFish; i++) {
    if (distance(vec3((*fishLoc)[i][0], (*fishLoc)[i][1], DEPTH), vec3(mx, my, DEPTH)) > AFFECTOR_THRESH) {
      averageHeading = getAverageHeading(i, numFish, *fishLoc, *headLoc);
      averagePosition = getAveragePosition(i, numFish, *fishLoc, *headLoc);
      averageDirection = normalize(averageHeading + averagePosition);
      averageDirection = (vec3) averageDirection / float (2.0);
      dist = distance(vec3((*fishLoc)[i][0], (*fishLoc)[i][1], 0), averageDirection);
      velocity.x = (SPEED / dist) * (averageDirection.x - (*fishLoc)[i][0]);
      velocity.y = (SPEED / dist) * (averageDirection.y - (*fishLoc)[i][1]);
      (*headLoc)[i][0] += velocity.x * dt;
      (*headLoc)[i][1] += velocity.y * dt;
    }
    else {
      dist = distance(vec3((*fishLoc)[i][0], (*fishLoc)[i][1], DEPTH), vec3(mx,my,DEPTH));
      velocity.x = (SPEED * 2 / dist) * (mx - (*fishLoc)[i][0]);
      velocity.y = (SPEED * 2 / dist) * (my - (*fishLoc)[i][1]);
    }
    velocity = avoidCollision(i, numFish, velocity, *fishLoc, *headLoc, groundH);
    (*fishLoc)[i][0] += velocity.x * dt;
    (*fishLoc)[i][1] += velocity.y * dt;
  }
  /*if (rand() % 1000 < 1) {
      for (int i = 0; i < numFish; i++) {
        (*headLoc)[i][0] = DEPTH + (((float)rand()/(float)(RAND_MAX)) * 5.0); 
        (*headLoc)[i][1] = (((float)rand()/(float)(RAND_MAX)) * 5.0);  
      }
   }*/
  
}

static float getCursorCoord(float cord, float depth, int dimen) {
  return (depth * ((2 * (cord / (float) dimen)) - 1));
}


#pragma GCC diagnostic pop