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

  vec3 trans;
};

class LinearScaleAnim : public AnimationComponent{
 public:
  LinearScaleAnim();
  LinearScaleAnim(const vec3 &dscale) : scalar(dscale){}

  vec3 scalar;
};

class LinearRotationAnim : public AnimationComponent{
 public:
  LinearRotationAnim() : mode(AXISANGLE), axis(vec3(0.0)), angle(0.0) {}
  LinearRotationAnim(const vec3 &axis, float dtheta) : axis(axis), angle(dtheta), mode(AXISANGLE) {}
  LinearRotationAnim(const vec3 &euler) : euler(euler), mode(EULER){}

  enum Mode{
    EULER,
    AXISANGLE
  };

  Mode mode;
  vec3 axis;
  float angle; // Saved rotation amount since quaternions only store orientation and won't 
  vec3 euler;
};

class AnimatableMesh : public AnimationComponent{
 public:
  AnimatableMesh();
  AnimatableMesh(vector<SolidMesh *> &copymesh){meshes = copymesh;}
  AnimatableMesh(vector<SolidMesh *> &copymesh, double timePerKeyFrame) :
    meshes(copymesh), timePerKeyFrame(timePerKeyFrame) { };
  ~AnimatableMesh(){};

  vector<SolidMesh *> meshes;
  int index = 0;
  double timePerKeyFrame = 0.1;
  double dtLastKeyFrame = 0;

  SolidMesh *getCurrentMesh() const {
    return meshes[index];
  }
};

#endif
