#include "AnimationSystem.hpp"
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
#include "components/AnimationComponents.hpp"


// Disable complaints about all the placeholder functions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#define GATHER_SINGLE_COMPONENT(_PT, _TYPE, _SRC) ( (_PT) = (((_PT) != NULL || dynamic_cast<_TYPE>((_SRC)) == NULL) ? (_PT) : static_cast<_TYPE>((_SRC))) )

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // Forward Declarations
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// draw Functions start general and get more specific

static void updateLinearAnimations(Entity* entity, double dt);

static void runLinearAnim(Component* linear, Pose* pose, double dt);

static void updateAnimatableMesh(AnimatableMesh *anim, double dt);

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
        // AnimationSystem functions
// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void AnimationSystem::update(ApplicationState &appstate, GameState &gstate, double elapsedTime){
  for(pair<const Entity*, Entity*> entpair : gstate.activeScene->entities){
    updateLinearAnimations(entpair.second, elapsedTime);
  }
}

static void updateLinearAnimations(Entity* entity, double dt){
  Pose* pose = NULL;
  vector<AnimationComponent*> animcomps;
  for(Component *cmpnt : entity->components){
    GATHER_SINGLE_COMPONENT(pose, Pose*, cmpnt);
    if(dynamic_cast<AnimationComponent*>(cmpnt)){
      animcomps.emplace_back(static_cast<AnimationComponent*>(cmpnt));
    }
    if(dynamic_cast<AnimatableMesh*>(cmpnt)){
      updateAnimatableMesh(static_cast<AnimatableMesh*>(cmpnt), dt);
    }
  }
  if(pose){
    for(AnimationComponent* pt : animcomps){
      runLinearAnim(pt, pose, dt);
    }
  }
}

static void updateAnimatableMesh(AnimatableMesh *anim, double dt) {
  anim->dtLastKeyFrame += dt;
  anim->updateKeyFrame();
}

static void runLinearAnim(Component* linear, Pose* pose, double dt){
  if(dynamic_cast</*const*/ LinearTranslationAnim*>(linear)){
    /*const*/ LinearTranslationAnim* translate = static_cast</*const*/ LinearTranslationAnim*>(linear);
    pose->loc += translate->trans * static_cast<float>(dt);
  }else if(dynamic_cast</*const*/ LinearRotationAnim*>(linear)){
    /*const*/ LinearRotationAnim* rot = static_cast</*const*/ LinearRotationAnim*>(linear);
    if(rot->mode == LinearRotationAnim::AXISANGLE){
      pose->orient *= glm::angleAxis(rot->angle * static_cast<float>(dt), rot->axis);
    }else{
      pose->orient *= glm::quat(rot->euler * static_cast<float>(dt));
    }
  }else if(dynamic_cast</*const*/ LinearScaleAnim*>(linear)){
    /*const*/ LinearScaleAnim* scale = static_cast</*const*/ LinearScaleAnim*>(linear);
    pose->scale += scale->scalar * static_cast<float>(dt);
  }
}


#pragma GCC diagnostic pop
