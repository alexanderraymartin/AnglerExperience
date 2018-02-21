#pragma once
#ifndef _SIMPLECOMPONENTS_H_
#define _SIMPLECOMPONENTS_H_

#include <vector>
#include <common.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "../Component.hpp"
#include "Geometry.hpp"
#include "Material.hpp"

using namespace std;

// This file is for components so simple they can be written as just a header

class Pose : public Component{
 public:
  Pose();
  Pose(const glm::vec3 nloc) : loc(nloc){};
  Pose(const glm::vec3 nloc, const glm::quat nori): loc(nloc), orient(nori){};

  glm::mat4 getAffineMatrix(){return(glm::translate(glm::mat4(1.0), loc) * glm::toMat4(orient) * glm::scale(glm::mat4(1.0), scale));}

  glm::vec3 loc = glm::vec3(0.0);
  glm::quat orient;
  glm::vec3 scale = glm::vec3(1.0);
};

class SolidMesh : public Component{
 public:
  SolidMesh();
  SolidMesh(vector<Geometry> &copygeom){geometries = copygeom;}
  ~SolidMesh(){};

  void setMaterial(Material &mat) {for(Geometry &geom : geometries){geom.material = mat;}}

  vector<Geometry> geometries;
};

class Camera : public Component{
 public:
  virtual ~Camera(){};
  virtual glm::vec3 getViewDir() = 0;
  virtual glm::mat4 getView() = 0;
  virtual glm::mat4 getPerspective(double aspect) = 0;

};

class StaticCamera : public Camera{
 public:
  StaticCamera() : fov(45.0), near(.01), far(100.0), pose(glm::vec3(0.0)), lookat(glm::vec3(0.0)), updir(glm::vec3(0.0, 1.0, 0.0)){}
  StaticCamera(double fov, const glm::vec3 &loc, const glm::vec3 &look) : fov(fov), near(.01), far(100.0), pose(loc), lookat(look), updir(glm::vec3(0.0, -1.0, 0.0)){}

  glm::vec3 getViewDir(){return(lookat-pose.loc);}

  glm::mat4 getView(){
    return(glm::lookAt(
      pose.loc,
      lookat,
      updir
    ));
  }

  glm::mat4 getPerspective(double aspect){
    return(glm::perspective(fov, aspect, near, far));
  }

  double fov;
  double near;
  double far;

  Pose pose;
  glm::vec3 lookat;
  glm::vec3 updir;
};

#endif
