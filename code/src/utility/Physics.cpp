
#include "Physics.h"
using namespace glm;

void witcherCamera::update(GLFWwindow *window, float dt) {
	userForce = vec3(0.0f);
	if (glfwGetKey(window, GLFW_KEY_W)) {
		userForce += vec3(INPUT_FORCE, 0.0f, 0.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_S)) {
		userForce += vec3(-INPUT_FORCE, 0.0f, 0.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_A)) {
		userForce += vec3(0.0f, INPUT_FORCE, 0.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_D)) {
		userForce += vec3(0.0f, -INPUT_FORCE, 0.0f);
	}
	applyForces(dt);
}

void witcherCamera::applyForces(float dt) {
	vec3 springForce = SPRING_CONSTANT * dirToCenter() - FRICTION_CONSTANT * calcVelocity();
	vec3 totalForce = springForce + userForce;

	//Do the force to velocity change TODO
	vec3 velocity = totalForce;

	viewDirection += velocity * dt;
}

vec3 witcherCamera::dirToCenter() {
	return defaultRotation - viewDirection;
}

//TODO
vec3 witcherCamera::calcVelocity() {
	return vec3(0.0f);
}