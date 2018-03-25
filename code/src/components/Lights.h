#pragma once
#ifndef __LIGHTS_H_
#define __LIGHTS_H_

#include "../Component.hpp"

class PointLight : public Component {
public:
	PointLight(glm::vec3 position, glm::vec3 color) : color(color), position(position) {}

  PointLight* clone() { return(new PointLight(*this)); }

	const glm::vec3 getPosition() { return position; }
	const glm::vec3 getColor() { return color; }
	const float getRadius() { return radius; }
	void setPosition(glm::vec3 pos) { position = pos; }

protected:
	glm::vec3 position;
	glm::vec3 color;
	float radius = 20.0f;
};

class SunLight : public Component {
public:
	SunLight(glm::vec3 color, glm::vec3 direction) : color(color), direction(glm::normalize(direction)), location(glm::vec3(0.0f)) {}
  SunLight(const glm::vec3& color, const glm::vec3& direction, const glm::vec3& location) : color(color), direction(glm::normalize(direction)), location(location) {}

  SunLight* clone() { return(new SunLight(*this)); }

  const glm::vec3& getLocation() { return location; }
  const glm::vec3& getColor() { return color; }
	const glm::vec3& getDirection() { return direction; }

  void setLocation(const glm::vec3& loc) { location = loc; }
	void setDirection(glm::vec3 dir) { direction = glm::normalize(dir); }

  glm::vec3 location; // Typically unesscessary but used for caustics. 
	glm::vec3 color;
	glm::vec3 direction;
};














#endif
