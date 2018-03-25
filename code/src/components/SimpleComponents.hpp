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
  Pose() {}
  Pose(const glm::vec3 nloc) : loc(nloc){};
  Pose(const glm::vec3 nloc, const glm::quat nori): loc(nloc), orient(nori){};

  Pose* clone() { return(new Pose(*this)); }

  glm::mat4 getAffineMatrix(){return(glm::translate(glm::mat4(1.0), loc) * glm::toMat4(orient) * glm::scale(glm::mat4(1.0), scale) * preaffine);}

  glm::vec3 loc = glm::vec3(0.0);
  glm::quat orient;
  glm::vec3 scale = glm::vec3(1.0);

  glm::mat4 preaffine = glm::mat4(1.0);
};

class SolidMesh : public Component{
 public:
  SolidMesh(vector<Geometry> &copygeom){geometries = copygeom;}
  ~SolidMesh(){};

  SolidMesh* clone() { return(new SolidMesh(*this)); }

  Component* instantiate() {
    SolidMesh* newmesh = new SolidMesh(geometries);
    return(newmesh);
  }

  void setMaterial(Material &mat) {for(Geometry &geom : geometries){geom.material = mat;}}

  vector<Geometry> geometries;
};

// This serves simply to seperate the ground plane from standard meshes
class GroundPlane : public Component {
 public:
  GroundPlane(const vec3& origin, const quat& orient, const vec3& scale) : levelOrigin(origin), levelOrient(orient), levelScale(scale) {}
  GroundPlane* clone() { return(new GroundPlane(*this)); }

  glm::mat4 getAffineMatrix(float levelProgress, int offset){
    return(
      glm::translate(glm::mat4(1.0), levelOrigin) * 
      glm::toMat4(levelOrient) * 
      glm::translate(glm::mat4(1.0), vec3(levelScale.x*2.00f + - mod(levelProgress+(levelScale.x*2.0f*(!offset)), levelScale.x*4.0f), 0.0f, 0.0f)) * 
      glm::scale(glm::mat4(1.0), levelScale)
    );
  }
 private:
  glm::vec3 levelOrigin;
  glm::quat levelOrient;
  glm::vec3 levelScale;
};

class Camera : public Component{
 public:
  virtual ~Camera(){};
  virtual glm::vec3 getLocation() = 0;
  virtual glm::vec3 getViewDir() = 0;
  virtual glm::mat4 getView() = 0;
  virtual glm::mat4 getPerspective(float aspect) = 0;

};

class Swarmable : public Component{
 public:
  virtual ~Swarmable(){};

  Swarmable* clone() { return new Swarmable(*this); }

  int fishNum;
};

class StaticCamera : public Camera{
 public:
  StaticCamera() : fov(45.0), near(.01), far(100.0), pose(glm::vec3(0.0)), lookat(glm::vec3(0.0)), updir(glm::vec3(0.0, 1.0, 0.0)){}
  StaticCamera(float fov, const glm::vec3 &loc, const glm::vec3 &look) : fov(fov), near(.01), far(100.0), pose(loc), lookat(look), updir(glm::vec3(0.0, -1.0, 0.0)){}
  StaticCamera* clone() { return(new StaticCamera(*this)); }

  glm::vec3 getViewDir(){return(lookat-pose.loc);}

  glm::vec3 getLocation() {
    return(pose.loc);
  }

  glm::mat4 getView(){
    return(glm::lookAt(
      pose.loc,
      lookat,
      updir
    ));
  }

  glm::mat4 getPerspective(float aspect){
    return(glm::perspective(glm::radians(fov), aspect, near, far));
  }

  float fov;
  float near;
  float far;

  Pose pose;
  glm::vec3 lookat;
  glm::vec3 updir;
};

#endif
