#include "Material.hpp"
#include <fstream>
#include <json.hpp>
#include <array>
#include <iostream>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

Material::Material(Program* shader) : shader(shader){
  // Nothing to do
}
Material::Material(const json &matjson, Program* shader) : shader(shader){
  loadFromJSON(matjson);
}

Material::Material(const json &matjson, ShaderLibrary &shaderlib){
  loadFromJSON(matjson);
  resolveShader(shaderlib);
}

Material::Material(const string &path, ShaderLibrary &shaderlib){
  ifstream matfile = ifstream(path);
  json tmp;
  if(matfile.is_open()){
    matfile >> tmp;
    matfile.close();
    cout << "Loaded " << path << endl;
    loadFromJSON(tmp);
    resolveShader(shaderlib);
  }else{
    cerr << "Couldn't open material file " << path << endl;
    shadername = "___missing___";
  }
}

void Material::resolveShader(ShaderLibrary &shaderlib){
  if(shader == NULL && !shadername.empty()){
    shader = shaderlib.getPtr(shadername);
  }else{
    shader = shaderlib.getPtr("");
  }
}

void Material::setIntProp(const string &keyword, int prop){
  internalMaterial[keyword] = {Material::Prop_Type_Int, prop};
}
void Material::setFloatProp(const string &keyword, float prop){
  internalMaterial[keyword] = {Material::Prop_Type_Float, prop};
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


void Material::applyIndividual(const string &key, const json &value, Material::TypeEnum type) const{
  switch(type){
    case Prop_Type_Int:
      glUniform1i(shader->getUniform(key), value.get<int>());
      break;
    case Prop_Type_Float:
      glUniform1f(shader->getUniform(key), value.get<float>());
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

void Material::apply() const{
  for(json::const_iterator it = internalMaterial.begin(); it != internalMaterial.end(); ++it){
    applyIndividual(it.key(), it.value()[1], it.value()[0].get<Material::TypeEnum>());
  }
}

void Material::exportJSON(ostream &outstream) const{
  json tmp = {{"shader", shadername}, {"material", internalMaterial}};
}

void Material::loadFromJSON(const json &matjson){
  if(matjson.find("shader") != matjson.end() && matjson.find("material") != matjson.end()){ // All good
    shadername = matjson["shader"].get<string>();
    internalMaterial = matjson["material"];
  }else if(matjson.find("shader") != matjson.end()){ // Shader name given but not material
    shadername = matjson["shader"].get<string>();
  }else if(matjson.find("material") != matjson.end()){ // Material object given but not shader name
    shadername = "___missing___";
  }else if(matjson.is_object()){ // Just an object, assume it's an unlabeled material
    shadername = "___missing___";
    internalMaterial = matjson;
  }else{
    fprintf(stderr, "Warning! Json object given for material appears to be malformed\n");
  }
}