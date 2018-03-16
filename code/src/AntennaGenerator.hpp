#pragma once
#ifndef ANTENNAGENERATOR_H_
#define ANTENNAGENERATOR_H_

#include <vector>
#include <glm/glm.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

#include "components/Geometry.hpp"

using namespace glm;
using namespace std;

class AntennaGenerator {
public:
  AntennaGenerator();
  AntennaGenerator(unsigned int rings, unsigned int ringVertices, float radius);

  Geometry* generateAntenna(vec3 origin, vec3 dest);

  vector<float> vertexBuffer;
  vector<float> normalBuffer;
  vector<unsigned int> indexBuffer;

private:
  unsigned int rings;
  unsigned int ringVertices;
  float radius;
  Geometry* geo;

  void generateVertices(vector<vec3> points, int rings, int ringVertices, float radius);
  void initFaces(int rings, int ringVertices);
  vec3 getBezierPoint(float delta, int i, vec3 P0, vec3 P1, vec3 P2, vec3 P3);
  vector<vec3> generateBezierLine(vec3 origin, vec3 dest, int points);

};

#endif

