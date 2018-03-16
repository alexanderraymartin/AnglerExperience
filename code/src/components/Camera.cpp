
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
	if (glfwGetKey(window, GLFW_KEY_R)) {
		shake(3.0f, 2.0f);
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
	updateShake(dt);
	applyForces(dt);
}

//This currently is using linear springs, not angular, but it has the nice side effect of not bringing the view outside of a 180° hemisphere.
void DynamicCamera::applyForces(float dt) {
	//F = -kx + bv
	//k = spring constant, x = spring direction, b = spring friction, v = velocity
	vec3 springForce = SPRING_CONSTANT * dirToCenter() - FRICTION_CONSTANT * velocity;
	//sum forces
	vec3 totalForce = springForce + userForce + shakeForce();
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

glm::vec3 DynamicCamera::getLocation() {
  return defaultLocation;
}

glm::mat4 DynamicCamera::getView() {
	return glm::lookAt(
		defaultLocation, //Location of the camera
		defaultLocation + viewDirection, //What to look at
		vec3(0, 1.0f, 0) //The direction of up
	);
}

void DynamicCamera::shake(float amount, float duration) {
	shakeDir = randomDir();
	shakeAmount = shakeBaseForce = amount;
	shakeDuration = duration;
	shakeTimePassed = 0;
}

vec3 DynamicCamera::randomDir() {
	uniform_real_distribution<float> distributionUniform(-1.0f, 1.0f);
	glm::vec3 dir = vec3(distributionUniform(generator), distributionUniform(generator), 0.0f);
	return normalize(dir);
}

void DynamicCamera::updateShake(float dt) {
	shakeTimePassed += dt;
	float newAmt = shakeBaseForce * (1 - shakeTimePassed / shakeDuration);
	if (newAmt > 0) {
		//This uses int casting and integer addition to change the direction of the camera shake SHAKE_CHANGE_RATE times per second
		if (((int)(shakeTimePassed * SHAKE_CHANGE_RATE)) - (int)((shakeTimePassed - dt) * SHAKE_CHANGE_RATE) != 0) {
			shakeDir = randomDir();
		}
		shakeAmount = newAmt;
	}
	else {
		shakeAmount = 0;
	}
}

glm::mat4 DynamicCamera::getPerspective(float aspect) {
	return(perspective(glm::radians(fov), aspect, near, far));
}

vec3 DynamicCamera::dirToCenter() {
	return defaultRotation - viewDirection;
}
