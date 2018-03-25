#pragma once
#ifndef _ANIMATIONCOMPONENTS_H_
#define _ANIMATIONCOMPONENTS_H_

#include <common.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "SimpleComponents.hpp"
#include "../Component.hpp"

using namespace std;
using namespace glm;

// Empty class used to quickly identify animation components later.
class AnimationComponent : public Component{};

class LinearTranslationAnim : public AnimationComponent{
 public:
  LinearTranslationAnim() : trans(vec3(0.0)){}
  LinearTranslationAnim(const vec3 &dxyz) : trans(dxyz){}
  LinearTranslationAnim* clone() { return(new LinearTranslationAnim(*this)); }

  vec3 trans;
};

class LinearScaleAnim : public AnimationComponent{
 public:
  LinearScaleAnim();
  LinearScaleAnim(const vec3 &dscale) : scalar(dscale){}
  LinearScaleAnim* clone() { return(new LinearScaleAnim(*this)); }

  vec3 scalar;
};

class LinearRotationAnim : public AnimationComponent{
 public:
  LinearRotationAnim() : mode(AXISANGLE), axis(vec3(0.0)), angle(0.0) {}
  LinearRotationAnim(const vec3 &axis, float dtheta) : axis(axis), angle(dtheta), mode(AXISANGLE) {}
  LinearRotationAnim(const vec3 &euler) : euler(euler), mode(EULER){}
  LinearRotationAnim* clone() { return(new LinearRotationAnim(*this)); }

  enum Mode{
    EULER,
    AXISANGLE
  };

  Mode mode;
  vec3 axis;
  float angle; // Saved rotation amount since quaternions only store orientation and won't 
  vec3 euler;
};

// Helper class for AnimatableMesh
class Animation {
 public:
  Animation();
  Animation(vector<SolidMesh *> & copymeshes) :
    meshes(copymeshes), timePerKeyFrame(0.1) {}
  Animation(vector<SolidMesh *> &copymeshes, double timePerKeyFrame) :
    meshes(copymeshes), timePerKeyFrame(timePerKeyFrame) {}

  Animation* clone() { return(new Animation(*this)); }

  vector<SolidMesh *> meshes;
  double timePerKeyFrame = 0.1;
  int meshIndex = 0;
  bool loop = true;
  bool stop = false;

  SolidMesh *getCurrentMesh() {
    return meshes[meshIndex];
  }
  SolidMesh *getNextMesh() {
    if (meshIndex + 1 >= meshes.size() && (!loop || stop)) {
        meshIndex = 0;
        stop = false;
	return NULL;
    }
    return meshes[(meshIndex + 1) % meshes.size()];
  }
  void stopAnimation() {
    stop = true;
  }
  void incrementKeyFrame() {
    meshIndex = (meshIndex + 1) % meshes.size();
  }
};

class AnimatableMesh : public AnimationComponent{
 public:
  AnimatableMesh();
  AnimatableMesh(Animation *anim){animations.push_back(anim);}
  ~AnimatableMesh(){};
  AnimatableMesh* clone() { return(new AnimatableMesh(*this)); }

  void addAnimation(Animation *anim) {animations.push_back(anim);}

  vector<Animation *> animations;
  int animationIndex = 0;
  int defaultAnimation = 0;
  double dtLastKeyFrame = 0;
  int nextAnimationIndex = 0;
  int changeAnimationIndex = -1;

  SolidMesh *getCurrentMesh() {
    return animations[animationIndex]->getCurrentMesh();
  }
  SolidMesh *getNextMesh() {
    SolidMesh *next = animations[nextAnimationIndex]->getNextMesh();
    if (next == NULL) {
      /* Not supposed to loop, return to default animation */
      if (changeAnimationIndex == -1) {
        nextAnimationIndex = defaultAnimation;
        next = animations[nextAnimationIndex]->getNextMesh();
      } else { /* Another animation was triggered */
        nextAnimationIndex = changeAnimationIndex;
        changeAnimationIndex = -1;
        next = animations[nextAnimationIndex]->getNextMesh();
      }
    }
    return next;
  }
  void updateKeyFrame() {
    if (dtLastKeyFrame > timeForKeyFrame()) {
      animations[animationIndex]->incrementKeyFrame();
      dtLastKeyFrame = 0;
    }
  }
  void triggerAnimation(int i) {
    changeAnimationIndex = i;
    animations[animationIndex]->stopAnimation();
  }
  double timeForKeyFrame() {
    return animations[animationIndex]->timePerKeyFrame;
  }
};

#endif
