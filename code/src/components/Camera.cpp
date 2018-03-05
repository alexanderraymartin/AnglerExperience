
#include <iostream>
#include "Camera.h"
using namespace glm;
using namespace std;

void DynamicCamera::update(GLFWwindow* window, float dt) {
	userForce = vec3(0.0f);
	if (glfwGetKey(window, GLFW_KEY_W)) {
		userForce += vec3(0.0f, INPUT_FORCE, 0.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_S)) {
		userForce += vec3(0.0f, -INPUT_FORCE, 0.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_A)) {
		userForce += vec3(INPUT_FORCE, 0.0f, 0.0f);
	}
	if (glfwGetKey(window, GLFW_KEY_D)) {
		userForce += vec3(-INPUT_FORCE, 0.0f, 0.0f);
	}
	if (userForce != vec3(0.0)) {
		userForce = normalize(userForce) * INPUT_FORCE;
	}
	/* Uncomment this for dynamic testing of physics variables
	if (glfwGetKey(window, GLFW_KEY_1)) {
		INPUT_FORCE += 1.0f;
		cout << "Input force: "  << INPUT_FORCE << "\n";
	}
	if (glfwGetKey(window, GLFW_KEY_2)) {
		SPRING_CONSTANT += 1.0f;
		cout << "Spring constant: " << SPRING_CONSTANT << "\n";
	}
	if (glfwGetKey(window, GLFW_KEY_3)) {
		FRICTION_CONSTANT += 1.0f;
		cout << "Friction constant: " << FRICTION_CONSTANT << "\n";
	}
	if (glfwGetKey(window, GLFW_KEY_4)) {
		CAMERA_MASS += 1.0f;
		cout << "Camera mass: " << CAMERA_MASS << "\n";
	}
	if (glfwGetKey(window, GLFW_KEY_Z)) {
		INPUT_FORCE -= 1.0f;
		cout << "Input force: " << INPUT_FORCE << "\n";
	}
	if (glfwGetKey(window, GLFW_KEY_X)) {
		SPRING_CONSTANT -= 1.0f;y
		cout << "Spring constant: " << SPRING_CONSTANT << "\n";
	}
	if (glfwGetKey(window, GLFW_KEY_C)) {
		FRICTION_CONSTANT -= 1.0f;
		cout << "Friction constant: " << FRICTION_CONSTANT << "\n";
	}
	if (glfwGetKey(window, GLFW_KEY_V)) {
		CAMERA_MASS -= 1.0f;
		cout << "Camera mass: " << CAMERA_MASS << "\n";
	}*/
	applyForces(dt);
}

//This currently is using linear springs, not angular, but it has the nice side effect of not bringing the view outside of a 180° hemisphere.
void DynamicCamera::applyForces(float dt) {
	//F = -kx + bv
	//k = spring constant, x = spring direction, b = spring friction, v = velocity
	vec3 springForce = SPRING_CONSTANT * dirToCenter() - FRICTION_CONSTANT * velocity;
	//sum forces
	vec3 totalForce = springForce + userForce;
	//F=MA or A = F/M
	vec3 acceleration = totalForce / CAMERA_MASS;

	//V1 = V0 + A*t
	velocity = velocity + acceleration * dt;

	//P1 = P0 + v*t
	viewDirection = viewDirection + velocity * dt;
	viewDirection = normalize(viewDirection);
}

glm::vec3 DynamicCamera::getViewDir() {
	return viewDirection;
}

glm::mat4 DynamicCamera::getView() {
	return glm::lookAt(
		defaultLocation, //Location of the camera
		defaultLocation + viewDirection, //What to look at
		vec3(0, 1.0f, 0) //The direction of up
	);
}

glm::mat4 DynamicCamera::getPerspective(float aspect) {
	return(perspective(glm::radians(fov), aspect, near, far));
}

vec3 DynamicCamera::dirToCenter() {
	return defaultRotation - viewDirection;
}