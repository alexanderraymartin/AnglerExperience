#pragma once
#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_obj_loader.h>
#include <common.h>

#include "../Component.hpp"


// This class 
class Geometry : public Component{
public:
  Geometry();
  Geometry(tinyobj::shape_t &shape);
  Geometry(const char* objname);
  virtual ~Geometry();

  void createGeometry(tinyobj::shape_t &shape);
  void init();
  void measure();
  void normalize();
  void adjust(glm::vec3 offset, float scale);

  static void loadFullObj(const char* objname, vector<Geometry> &geometrysequence);
  // void draw(const std::shared_ptr<Program> shader) const;

  glm::vec3 min;
  glm::vec3 max;

  std::shared_ptr<std::vector<unsigned int>> eleBuf;
  std::shared_ptr<std::vector<float>> posBuf;
  std::shared_ptr<std::vector<float>> norBuf;
  std::shared_ptr<std::vector<float>> texBuf;
  unsigned eleBufID;
  unsigned posBufID;
  unsigned norBufID;
  unsigned texBufID;
  unsigned vaoID;
};

#endif
