#include "Geometry.hpp"
#include <iostream>
#include <assert.h>

#define GLEW_STATIC
#include <GL/glew.h>

#include <tiny_obj_loader.h>

using namespace std;
using namespace glm;

Geometry::Geometry() :
  eleBufID(0),
  posBufID(0),
  norBufID(0),
  texBufID(0), 
   vaoID(0)
{
  min = glm::vec3(0);
  max = glm::vec3(0);
}

Geometry::Geometry(tinyobj::shape_t &shape){
  createGeometry(shape);
}

Geometry::Geometry(const char* objname){
  vector<tinyobj::shape_t> TOshapes;
  vector<tinyobj::material_t> objMaterials;
  string errStr;

  bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, objname);
  if(!rc) {
    cerr << errStr << endl;
  }else{
    vec3 Gmin, Gmax;
    Gmin = vec3(0.0);
    Gmax = vec3(0.0);
    if(TOshapes.size() > 1){
      fprintf(stderr, "Warning!: Obj given to construct geometry contains multiple shapes! Only the first will be loaded. To avoid this error in the future use loadFullObj\n");
    }

    createGeometry(TOshapes[0]);
    
    normalize();
  }
}

Geometry::~Geometry(){
  
}

/* copy the data from the shape to this object */
void Geometry::createGeometry(tinyobj::shape_t &shape)
{
    posBuf = make_shared<vector<float>>(shape.mesh.positions);
    norBuf = make_shared<vector<float>>(shape.mesh.normals);
    texBuf = make_shared<vector<float>>(shape.mesh.texcoords);
    eleBuf = make_shared<vector<unsigned int>>(shape.mesh.indices);

}

void Geometry::normalize(){
  measure();

  vec3 dimension = max - min;
  float scale = 2.0/MAX(MAX(dimension.x, dimension.y), dimension.z);
  vec3 offset = -(max + min)*0.5f;

  adjust(offset, scale);
}

void Geometry::measure() {
  float minX, minY, minZ;
  float maxX, maxY, maxZ;

  minX = minY = minZ = 1.1754E+38F;
  maxX = maxY = maxZ = -1.1754E+38F;
 
  //Go through all vertices to determine min and max of each dimension
  for (size_t v = 0; v < posBuf->size() / 3; v++) {
    if((*posBuf)[3*v+0] < minX) minX = (*posBuf)[3*v+0];
    if((*posBuf)[3*v+0] > maxX) maxX = (*posBuf)[3*v+0];
 
    if((*posBuf)[3*v+1] < minY) minY = (*posBuf)[3*v+1];
    if((*posBuf)[3*v+1] > maxY) maxY = (*posBuf)[3*v+1];
 
    if((*posBuf)[3*v+2] < minZ) minZ = (*posBuf)[3*v+2];
    if((*posBuf)[3*v+2] > maxZ) maxZ = (*posBuf)[3*v+2];
}

  min.x = minX;
  min.y = minY;
  min.z = minZ;
  max.x = maxX;
  max.y = maxY;
  max.z = maxZ;
}

void Geometry::adjust(glm::vec3 offset, float scale){
  for(UINT v = 0; v < posBuf->size(); v+=3){
    (*posBuf)[v]= ((*posBuf)[v] + offset.x)*scale;
    (*posBuf)[v+1]= ((*posBuf)[v+1] + offset.y)*scale;
    (*posBuf)[v+2]= ((*posBuf)[v+2] + offset.z)*scale;
  }
  min = (min+offset) * scale;
  max = (max+offset) * scale;
}

void Geometry::init()
{
  // Initialize the vertex array object
  glGenVertexArrays(1, &vaoID);
  glBindVertexArray(vaoID);

  // Send the position array to the GPU
  glGenBuffers(1, &posBufID);
  glBindBuffer(GL_ARRAY_BUFFER, posBufID);
  glBufferData(GL_ARRAY_BUFFER, posBuf->size()*sizeof(float), &(*posBuf)[0], GL_STATIC_DRAW);
  
  // Send the normal array to the GPU
  if(norBuf->empty()) {
    norBufID = 0;
  } else {
    glGenBuffers(1, &norBufID);
    glBindBuffer(GL_ARRAY_BUFFER, norBufID);
    glBufferData(GL_ARRAY_BUFFER, norBuf->size()*sizeof(float), &(*norBuf)[0], GL_STATIC_DRAW);
  }
  
  // Send the texture array to the GPU
  if(texBuf->empty()) {
    texBufID = 0;
  } else {
    glGenBuffers(1, &texBufID);
    glBindBuffer(GL_ARRAY_BUFFER, texBufID);
    glBufferData(GL_ARRAY_BUFFER, texBuf->size()*sizeof(float), &(*texBuf)[0], GL_STATIC_DRAW);
  }
  
  // Send the element array to the GPU
  glGenBuffers(1, &eleBufID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf->size()*sizeof(unsigned int), &(*eleBuf)[0], GL_STATIC_DRAW);
  
  // Unbind the arrays
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  
  assert(glGetError() == GL_NO_ERROR);
}

void Geometry::loadFullObj(const char* objname, vector<Geometry> &geometrysequence){
  vector<tinyobj::shape_t> TOshapes;
  vector<tinyobj::material_t> objMaterials;
  string errStr;

  bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, objname);
  if(!rc) {
    cerr << errStr << endl;
  }else{
    vec3 Gmin, Gmax;
    Gmin = vec3(0.0);
    Gmax = vec3(0.0);
    for(UINT i=0; i < TOshapes.size(); i++){
      geometrysequence.emplace_back(Geometry(TOshapes[i]));
      geometrysequence.back().measure();

      if(geometrysequence.back().min.x < Gmin.x) Gmin.x = geometrysequence.back().min.x;
      if(geometrysequence.back().min.y < Gmin.y) Gmin.y = geometrysequence.back().min.y;
      if(geometrysequence.back().min.z < Gmin.z) Gmin.z = geometrysequence.back().min.z;
      if(geometrysequence.back().max.x > Gmax.x) Gmax.x = geometrysequence.back().max.x;
      if(geometrysequence.back().max.y > Gmax.y) Gmax.y = geometrysequence.back().max.y;
      if(geometrysequence.back().max.z > Gmax.z) Gmax.z = geometrysequence.back().max.z;

    }

    vec3 dist = Gmax - Gmin;
    float objscale = 2.0/MAX(MAX(dist.x, dist.y), dist.z);
    vec3 objoffset = -(Gmax+Gmin)*0.5f;

    for(UINT i = 0; i < geometrysequence.size(); i++){
      geometrysequence[i].adjust(objoffset, objscale);
      geometrysequence[i].init();
      geometrysequence[i].sourceName = string(objname);
    }
  }
}

void Geometry::initDynamic()
{
  // Initialize the vertex array object
  glGenVertexArrays(1, &vaoID);
  glBindVertexArray(vaoID);

  // Send the position array to the GPU
  glGenBuffers(1, &posBufID);
  glBindBuffer(GL_ARRAY_BUFFER, posBufID);
  glBufferData(GL_ARRAY_BUFFER, posBuf->size()*sizeof(float), &(*posBuf)[0], GL_DYNAMIC_DRAW);

  // Send the normal array to the GPU
  if(norBuf->empty()) {
    norBufID = 0;
  } else {
    glGenBuffers(1, &norBufID);
    glBindBuffer(GL_ARRAY_BUFFER, norBufID);
    glBufferData(GL_ARRAY_BUFFER, norBuf->size()*sizeof(float), &(*norBuf)[0], GL_DYNAMIC_DRAW);
  }

  // Send the texture array to the GPU
  if(texBuf->empty()) {
    texBufID = 0;
  } else {
    glGenBuffers(1, &texBufID);
    glBindBuffer(GL_ARRAY_BUFFER, texBufID);
    glBufferData(GL_ARRAY_BUFFER, texBuf->size()*sizeof(float), &(*texBuf)[0], GL_DYNAMIC_DRAW);
  }

  // Send the element array to the GPU
  glGenBuffers(1, &eleBufID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf->size()*sizeof(unsigned int), &(*eleBuf)[0], GL_STATIC_DRAW);

  // Unbind the arrays
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  assert(glGetError() == GL_NO_ERROR);
}

void Geometry::update() {
  glBindVertexArray(vaoID);
  glBindBuffer(GL_ARRAY_BUFFER, posBufID);
  glBufferSubData(GL_ARRAY_BUFFER, 0, posBuf->size()*sizeof(float), &(*posBuf)[0]);

  if(!norBuf->empty()) {
    glBindBuffer(GL_ARRAY_BUFFER, norBufID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, norBuf->size()*sizeof(float), &(*norBuf)[0]);
  }

  if(!texBuf->empty()) {
    glBindBuffer(GL_ARRAY_BUFFER, texBufID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, texBuf->size()*sizeof(float), &(*texBuf)[0]);
  }
}

