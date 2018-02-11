#pragma once
#ifndef _MATERIALCOMPONENT_H_
#define _MATERIALCOMPONENT_H_

#include <istream>

#include <glm/glm.hpp>
#include <json.hpp>

#include <common.h>
#include "../Component.hpp"
#include <ShaderLibrary.hpp>
#include <Program.h>

using namespace std;
using namespace glm;


// Materials stored in JSON should be stored in the following format.

// {
//  "shader": "(ie 'blinn-phong')",
//  "material":{
//   "unifname": [Prop_Type_Vec2, [1.0,2.0]],
//   "otherunif": [Prop_Type_Vec3, [1.0, 3.2, 4.5]],
//   "unifint": [Prop_Type_Int, 1]
//  } 
// }
// Notice the type specified like Prop_Type_Vec2. These are integer values from the 'TypeEnum'
// below. They've been set to specific values to (hopefully) be memorable. Eventually we'll
// want to make a utility to make the materials in engine then export.

// 1 = int, 0x0F = float, 0x0[2-4] = vector of given dimension, 0x[2-4]0 = matrix of given dimension
// Unfortunately hex literals aren't allowed in JSON so you'll have to do the math


class Material : public Component{
 public:
  Material() {}
  Material(const json &matjson, ShaderLibrary &shaderlib);
  Material(Program* shader);
  Material(const string &shadername) : shadername(shadername) {}
  Material(const json &matjson, Program* shader);

  void setIntProp(const string &keyword, int prop);
  void setFloatProp(const string &keyword, float prop);
  void setVec2Prop(const string &keyword,const vec2 &prop);
  void setVec3Prop(const string &keyword,const vec3 &prop);
  void setVec4Prop(const string &keyword,const vec4 &prop);
  void setMat2Prop(const string &keyword,const mat2 &prop);
  void setMat3Prop(const string &keyword,const mat3 &prop);
  void setMat4Prop(const string &keyword,const mat4 &prop);

  // int getIntProp(const string &keyword);
  // float getFloatProp(const string &keyword);
  // vec2 getVec2Prop(const string &keyword);
  // vec3 getVec3Prop(const string &keyword);
  // vec4 getVec4Prop(const string &keyword);
  // mat2 getMat2Prop(const string &keyword);
  // mat3 getMat3Prop(const string &keyword);
  // mat4 getMat4Prop(const string &keyword);

  void apply() const;

  // If true the RenderSystem should check if the shader has a uniform before attempting to set it
  // This simply serves as a way to reduce warnings from OpenGL
  bool checkShaderFirst = true;

  string shadername;
  Program* shader = NULL;

 protected:
  enum TypeEnum{
    Prop_Type_Int = 1,
    Prop_Type_Float = 0x0F, // 15
    Prop_Type_Vec2 = 0x02,  // 2
    Prop_Type_Vec3 = 0x03,  // 3
    Prop_Type_Vec4 = 0x04,  // 4
    Prop_Type_Mat2 = 0x20,  // 32
    Prop_Type_Mat3 = 0x30,  // 48
    Prop_Type_Mat4 = 0x40   // 64
  };

  void applyIndividual(const string &key, const json &value, Material::TypeEnum type) const;

  void resolveShader(ShaderLibrary &shaderlib);
  void loadFromJSON(const json &matjson);

  json internalMaterial;

};


#endif