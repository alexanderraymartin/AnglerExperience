#pragma once
#ifndef _LIGHTINGCOMPONENTS_H_
#define _LIGHTINGCOMPONENTS_H_

#include <glm/glm.hpp>

#include <common.h>
#include "../Component.hpp"

using namespace std;
using namespace glm;

class LightingComponent : public Component{};

class PointLight : public LightingComponent{
 public:
  enum Falloff{INVERSESQUARE, LINEAR};
  PointLight();
  PointLight(const vec3 &color) : color(color) {}
  PointLight(float intensity, float distance) : intensity(intensity), distance(distance) {}
  PointLight(float intensity, float distance, PointLight::Falloff fall) : intensity(intensity), distance(distance), falloff(fall) {}
  PointLight(const vec3 &color, float intensity, float distance) : color(color), intensity(intensity), distance(distance) {}
  PointLight(const vec3 &color, float intensity, float distance, Falloff fall) : color(color), intensity(intensity), distance(distance), falloff(fall) {}

  float intensity = 1.0;
  float distance = 15.0;
  vec3 color = vec3(1.0);
  Falloff falloff = INVERSESQUARE;

};

class SunLight : public LightingComponent{
 public:
  SunLight();
  SunLight(const vec3 &color) : color(color) {}
  SunLight(const vec3 &color, const vec3 &direction, const vec3& location) : color(color), direction(direction), location(location) {}
  SunLight(const vec3 &color, const vec3 &direction, const vec3& location, float intensity) : color(color), direction(direction), location(location), intensity(intensity) {}

  vec3 color = vec3(1.0);
  vec3 direction = vec3(0.0, -1.0, 0.0);
  vec3 location = vec3(0.0);
  float intensity = 1.0;
};

#endif