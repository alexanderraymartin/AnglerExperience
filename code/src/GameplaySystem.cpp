#include "GameplaySystem.hpp"
#include <common.h>
#include <Program.h>

#include "core.h"
#include "GameState.hpp"
#include "Entity.hpp"

#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "components/Geometry.hpp"
#include "components/SimpleComponents.hpp"
#include "components/Lights.h"

using namespace std;

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // GameplaySystem functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void GameplaySystem::update(ApplicationState &appstate, GameState &gstate, double elapsedTime){
  gstate.levelProgress = gstate.gameTime.get() * 1.0/30.0;

  Entity* sun = gstate.activeScene->getFirstWithComponent<SunLight>();
  SunLight* suncmpnt = sun->getFirstComponentType<SunLight>();
  suncmpnt->setDirection(vec3(0.0, -1.0, 0.0) + -gstate.levelDirection*static_cast<float>(gstate.levelProgress));
}
