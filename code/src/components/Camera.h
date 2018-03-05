#pragma once
#ifndef CAMERA_H_
#define CAMERA_H_

#include "SimpleComponents.hpp"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <random>

class DynamicCamera : public Camera {
protected:
	//Random number generator
	default_random_engine generator;
	//The amount of force applied by the user, calculated in update based on keys pressed
	glm::vec3 userForce;
	//The direction the camera faces when the game begins
	const glm::vec3 defaultRotation = normalize(glm::vec3(0.0f, 0.0f, 1.0f));
	//The location of the camera when the game begins
	const glm::vec3 defaultLocation = glm::vec3(0.0f, 3.4f, 0.0f);
	//The direction the camera is facing
	glm::vec3 viewDirection;
	//The 3 angular velocities of the camera. Z can not change in our current implementation so the up-vector will not change.
	glm::vec3 velocity;

	//If you don't like these numbers, mess with them until you like them.
	//The amount of force the user pushes with
	float INPUT_FORCE = 1.0f;
	//The amount of force the camera tries to reset with
	float SPRING_CONSTANT = 2.0f;
	//The amount of friction on the snapback force
	float FRICTION_CONSTANT = 3.0f;
	//The more mass, the slower all accelerations are.
	float CAMERA_MASS = 1.0f;
	//The amount of force reduced per second
	float SHAKE_DECAY_RATE = 1.0f;
	//The amount of times per second that the shake direction should change
	int SHAKE_CHANGE_RATE = 15;

	//Field of view in degrees
	float fov;
	//The disntance to the near plane
	float near;
	//The distance to the far plane
	float far;

	//The amount of force used to shake the camera
	float shakeAmount;
	//The direction the shake force is pushing
	glm::vec3 shakeDir;

	//Calculates the physics on the camera based on forces affecting it.
	void applyForces(float dt);

	//The direction from where the camera currently is focused to the focus
	glm::vec3 dirToCenter();

	//Reduces the shake amount and sometimes changes the direction
	void updateShake(float dt);

	//Returns a random normalized direction
	glm::vec3 randomDir();

	glm::vec3 shakeForce() { return shakeAmount * shakeDir; }

public:

	DynamicCamera() : fov(35.0), near(.01f), far(100.0f), viewDirection(defaultRotation) {}

	DynamicCamera(float fov, float near, float far) : fov(fov), near(near), far(far), viewDirection(defaultRotation) {}

	//Shake the camera
	//Current model uses single instance of force
	//Amount is intensity and duration in seconds
	void shake(float amount);
	
	//Returns the direction the camera is facing
	glm::vec3 getViewDir();

	//Returns the view matrix
	glm::mat4 getView();

	//Returns the perspective matrix associated with the camera
	glm::mat4 getPerspective(float aspect);

	//Updates the camera
	void update(GLFWwindow* window, float dt);

};

#endif // !CAMERA_H_
