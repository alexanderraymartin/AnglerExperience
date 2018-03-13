#include "AntennaGenerator.hpp"

AntennaGenerator::AntennaGenerator() : rings(15), ringVertices(30), radius(0.1f) {
  cylinderVertexBuffer.resize((ringVertices * rings + 2) * 3);
  cylinderIndexBuffer.resize(ringVertices * rings * 6);

  initCylinderFaces(rings, ringVertices);
}

AntennaGenerator::AntennaGenerator(unsigned int _rings, unsigned int _ringVertices, float _radus) : rings(_rings), ringVertices(_ringVertices), radius(_radus) {
  cylinderVertexBuffer.resize((ringVertices * rings + 2) * 3);
  cylinderIndexBuffer.resize(ringVertices * rings * 6);

  initCylinderFaces(rings, ringVertices);
}

void AntennaGenerator::generateCylinderVertices(vector<vec3> points, int rings, int ringVertices, float radius) {

  cylinderVertexBuffer[0] = points[0].x;
  cylinderVertexBuffer[1] = points[0].y;
  cylinderVertexBuffer[2] =  points[0].z;

  cylinderVertexBuffer[ringVertices * rings * 3 + 3] = points[rings - 1].x;
  cylinderVertexBuffer[ringVertices * rings * 3 + 4] = points[rings - 1].y;
  cylinderVertexBuffer[ringVertices * rings * 3 + 5] = points[rings - 1].z;

  //Generate cylinder vertices
  for (int j = 0; j < rings; j++) {
    float angle;
    int baseIdx;

    vec3 axis;
    if (j == 0) {
      axis = points[j+1]-points[j];
    } else {
      axis = points[j]-points[j-1];
    }

    vec3 a = normalize(cross(axis, vec3(1.0,0.0,1.0)));
    vec3 b = normalize(cross(axis, a));

    for (int i = 0; i < ringVertices; i++) {
      angle = 2 * M_PI / ringVertices * i;
      baseIdx = 3 * (i + j * ringVertices + 1);

      cylinderVertexBuffer[baseIdx] = radius * (cos(angle) * a.x + sin(angle) * b.x) + points[j].x;
      cylinderVertexBuffer[baseIdx + 1] = radius * (cos(angle) * a.y + sin(angle) * b.y) + points[j].y;
      cylinderVertexBuffer[baseIdx + 2] = radius * (cos(angle) * a.z + sin(angle) * b.z) + points[j].z;
    }
  }
}

void AntennaGenerator::initCylinderFaces(int rings, int ringVertices) {

  int  baseIdx = 0;

  //Generate front face indices
  for (int i = 0; i < ringVertices; i++) {
    cylinderIndexBuffer[baseIdx++] = 0;
    cylinderIndexBuffer[baseIdx++] = i + 1;
    cylinderIndexBuffer[baseIdx++] = i + 2 > ringVertices ? i - ringVertices + 2: i + 2;
  }

  //Genereate cylinder indices
  int currVert;
  for (int i = 0; i < rings - 1; i++) {
    for (int j = 0; j < ringVertices; j++) {
      currVert = j + i * ringVertices + 1;

      cylinderIndexBuffer[baseIdx++] = currVert;
      cylinderIndexBuffer[baseIdx++] = currVert + ringVertices;
      cylinderIndexBuffer[baseIdx++] = j < ringVertices - 1 ? currVert + 1 : currVert - ringVertices + 1;
      cylinderIndexBuffer[baseIdx++] = currVert + ringVertices;
      cylinderIndexBuffer[baseIdx++] = j < ringVertices - 1 ? currVert + ringVertices + 1: currVert + 1;
      cylinderIndexBuffer[baseIdx++] = j < ringVertices - 1 ? currVert + 1 : currVert - ringVertices + 1;
    }
  }

  //Generate back face indices
  for (int i = 0; i < ringVertices; i++) {
    cylinderIndexBuffer[baseIdx++] = ringVertices * rings + 1;
    cylinderIndexBuffer[baseIdx++] = ringVertices * rings - ringVertices + i + 1;
    cylinderIndexBuffer[baseIdx++] = i + 2 > ringVertices ? ringVertices * rings - ringVertices * 2 + i + 2 : ringVertices * rings - ringVertices + i + 2;
  }
}

vec3 AntennaGenerator::getBezierPoint(float delta, int i, vec3 P0, vec3 P1, vec3 P2, vec3 P3) {
  float t = delta * i;
  float t2 = t * t;
  float one_minus_t = 1.0f - t;
  float one_minus_t2 = one_minus_t * one_minus_t;
  return (P0 * one_minus_t2 * one_minus_t + P1 * 3.0f * t * one_minus_t2 + P2 * 3.0f * t2 * one_minus_t + P3 * t2 * t);
}

vector<vec3> AntennaGenerator::generateBezierLine(vec3 origin, vec3 dest, int points) {
  vec3 oneThird = vec3(origin.x * 2.0f / 3.0f + dest.x / 3.0f, glm::max(origin.y, dest.y) + 0.25f, origin.z * 2.0f / 3.0f + dest.z / 3.0f);
  vec3 twoThirds = vec3(origin.x / 3.0f + dest.x * 2.0f / 3.0f, glm::max(origin.y, dest.y) + 0.5f, origin.z / 3.0f + dest.z * 2.0f / 3.0f);

  float delta = 1.0f / (points - 1);

  vector<vec3> linePoints;
  for (int i = 0; i < points; i++) {
    linePoints.push_back(getBezierPoint(delta, i, origin, oneThird, twoThirds, dest));
  }
  return linePoints;
}

void AntennaGenerator::generateAntenna(vec3 origin, vec3 dest) {
  vector<vec3> points = generateBezierLine(origin, dest, rings);
  generateCylinderVertices(points, rings, ringVertices, radius);
}
