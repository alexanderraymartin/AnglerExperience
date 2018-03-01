#pragma once
#ifndef PHYSICS_H_

#include "SimpleComponents.hpp"
#include <GLFW/glfw3.h>

class witcherCamera : Camera {
protected:
	glm::vec3 userForce;
	const glm::vec3 defaultRotation = glm::vec3(0.0f, 0.0f, 1.0f);
	glm::vec3 viewDirection;
	const float INPUT_FORCE = 1.0f;
	const float SPRING_CONSTANT = 1.0f;
	const float FRICTION_CONSTANT = 1.0f;

	void applyForces(float dt);

	glm::vec3 calcVelocity();

	glm::vec3 dirToCenter();

public:

	void update(GLFWwindow *window, float dt);

};

#endif // !PHYSICS_H_
