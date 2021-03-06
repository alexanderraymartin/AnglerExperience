#include "GameplaySystem.hpp"
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
#include "components/Lights.h"

#define SPEED .005f
#define SWARM_RADIUS .1f
#define FISH_RADIUS .05f
#define NUM_FISH 50
#define DEPTH 1.75
#define BAIT_RADIUS .1f
#define AFFECTOR_THRESH 1.5f

using namespace std;
using namespace glm;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Forward Declarations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// draw Functions start general and get more specific

static void checkEatZone(int fish, Entity* entity, int numFish, float ***fishLoc, float ***headLoc, float dt, GameState &gstate);

static void updateGame(Entity* entity, double dt, float ***fishLoc, float ***headLoc, GLFWwindow* window, GameState &gstate);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // GameplaySystem functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void GameplaySystem::update(ApplicationState &appstate, GameState &gstate, double elapsedTime, float ***fishLoc, float ***headLoc){
  gstate.levelProgress = gstate.gameTime.get() * 1.0/3.0;

  Entity* sun = gstate.activeScene->getFirstWithComponent<SunLight>();
  SunLight* suncmpnt = sun->getFirstComponentType<SunLight>();
  suncmpnt->setLocation(vec3(0.0, 10.0, 0.0) + -gstate.levelDirection*static_cast<float>(gstate.levelProgress));
  
  for(pair<const Entity*, Entity*> entpair : gstate.activeScene->entities){
    updateGame(entpair.second, elapsedTime, fishLoc, headLoc, appstate.window, gstate);
  }
}

static void updateGame(Entity* entity, double dt, float ***fishLoc, float ***headLoc, GLFWwindow* window, GameState &gstate) {
  Pose* pose = NULL;
  Swarmable* swarm = NULL;

  for(Component *cmpnt : entity->components){
    GATHER_SINGLE_COMPONENT(pose, Pose*, cmpnt);
    GATHER_SINGLE_COMPONENT(swarm, Swarmable*, cmpnt);
    if(pose && swarm){
      checkEatZone(swarm->fishNum, entity, NUM_FISH, fishLoc, headLoc, dt, gstate);
    }
  }

}

static void checkEatZone(int fish, Entity* entity, int numFish, float ***fishLoc, float ***headLoc, float dt, GameState &gstate) { // .56, 3.2
  if (distance(vec2((*fishLoc)[fish][0],(*fishLoc)[fish][1]), vec2(-.6, 3.17)) < .25f) {
    entity->isActive = false;
    gstate.activeScene->submitToKill(entity);
  }
}

