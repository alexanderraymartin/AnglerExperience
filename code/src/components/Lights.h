#pragma once
#ifndef __LIGHTS_H_
#define __LIGHTS_H_

#include "../Component.hpp"

class PointLight : public Component {
public:
	PointLight(glm::vec3 position, glm::vec3 color) : color(color), position(position) {}
	const glm::vec3 getPosition() { return position; }
	const glm::vec3 getColor() { return color; }
	const float getRadius() { return radius; }

protected:
	glm::vec3 position;
	glm::vec3 color;
	float radius = 20.0f;
};

class SunLight : public Component {
public:
	SunLight(glm::vec3 color, glm::vec3 direction) : color(color), direction(direction) {}
	const glm::vec3 getColor() { return color; }
	const glm::vec3 getDirection() { return direction; }

	void setDirection(glm::vec3 dir) { direction = dir; }

	glm::vec3 color;
	glm::vec3 direction;
};














#endif
