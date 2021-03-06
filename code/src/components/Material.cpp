#include "Material.hpp"
#include <fstream>
#include <json.hpp>
#include <array>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>

#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

Material::Material(const json &matjson){
  loadFromJSON(matjson);
}

Material::Material(const string &path){
  ifstream matfile = ifstream(path);
  json tmp;
  if(matfile.is_open()){
    matfile >> tmp;
    matfile.close();
    cout << "Loaded " << path << endl;
    loadFromJSON(tmp);
  }else{
    cerr << "Couldn't open material file " << path << endl;
  }
}

void Material::setCheckFirst(bool checkFirst){
  checkShaderFirst = checkFirst;
}

bool Material::getCheckFirst(){
  return(checkShaderFirst);
}

void Material::setIntProp(const string &keyword, int prop){
  internalMaterial[keyword] = {Material::Prop_Type_Int, prop};
}
void Material::setFloatProp(const string &keyword, float prop){
  internalMaterial[keyword] = {Material::Prop_Type_Float, prop};
}
void Material::setBoolProp(const string &keyword, bool prop){
  internalMaterial[keyword] = {Material::Prop_Type_Bool, prop};
}
void Material::setVec2Prop(const string &keyword, const vec2 &prop){
  internalMaterial[keyword] = {Material::Prop_Type_Vec2, {prop.x, prop.y}};
}
void Material::setVec3Prop(const string &keyword, const vec3 &prop){
  internalMaterial[keyword] = {Material::Prop_Type_Vec3, {prop.x, prop.y, prop.z}};
}
void Material::setVec4Prop(const string &keyword, const vec4 &prop){
  internalMaterial[keyword] = {Material::Prop_Type_Vec4, {prop.x, prop.y, prop.z, prop.w}};
}
void Material::setMat2Prop(const string &keyword, const mat2 &prop){
  vector<float> tmp(value_ptr(prop), value_ptr(prop) + 4);
  internalMaterial[keyword] = {Material::Prop_Type_Mat2, tmp};
}
void Material::setMat3Prop(const string &keyword, const mat3 &prop){
  vector<float> tmp(value_ptr(prop), value_ptr(prop) + 9);
  internalMaterial[keyword] = {Material::Prop_Type_Mat3, tmp};
}
void Material::setMat4Prop(const string &keyword, const mat4 &prop){
  vector<float> tmp(value_ptr(prop), value_ptr(prop) + 16);
  internalMaterial[keyword] = {Material::Prop_Type_Mat4, tmp};
}


void Material::applyIndividual(const string &key, const json &value, Material::TypeEnum type, Program* shader) const{
  switch(type){
    case Prop_Type_Int:
      glUniform1i(shader->getUniform(key), value.get<int>());
      break;
    case Prop_Type_Float:
      glUniform1f(shader->getUniform(key), value.get<float>());
      break;
    case Prop_Type_Bool:
      glUniform1i(shader->getUniform(key), value.get<bool>());
      break;
    case Prop_Type_Vec2:
      glUniform2fv(shader->getUniform(key), 1, value.get<array<GLfloat, 2>>().data());
      break;
    case Prop_Type_Vec3:
      glUniform3fv(shader->getUniform(key), 1, value.get<array<GLfloat, 3>>().data());
      break;
    case Prop_Type_Vec4:
      glUniform4fv(shader->getUniform(key), 1, value.get<array<GLfloat, 4>>().data());
      break;
    case Prop_Type_Mat2:
      glUniformMatrix2fv(shader->getUniform(key), 1, GL_FALSE, value.get<array<GLfloat, 4>>().data());
      break;
    case Prop_Type_Mat3:
      glUniformMatrix3fv(shader->getUniform(key), 1, GL_FALSE, value.get<array<GLfloat, 9>>().data());
      break;
    case Prop_Type_Mat4:
      glUniformMatrix4fv(shader->getUniform(key), 1, GL_FALSE, value.get<array<GLfloat, 16>>().data());
      break;
    default:
      break;
  }
}

void Material::apply(Program* shader) const{
  for(json::const_iterator it = internalMaterial.begin(); it != internalMaterial.end(); ++it){
    if( !checkShaderFirst || shader->hasUniform(it.key() )){
      applyIndividual(it.key(), it.value()[1], it.value()[0].get<Material::TypeEnum>(), shader);
    }
  }
}

void Material::exportJSON(ostream &outstream) const{
  json tmp = {{"material", internalMaterial}};
}

void Material::loadFromJSON(const json &matjson){
  if(matjson.find("material") != matjson.end()){ // All good
    internalMaterial = matjson["material"];
  }else if(matjson.is_object()){ // Just an object, assume it's an unlabeled material
    internalMaterial = matjson;
  }else{
    fprintf(stderr, "Warning! Json object given for material appears to be malformed\n");
  }
}