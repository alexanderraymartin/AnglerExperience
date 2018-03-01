#pragma once
#ifndef LAB476_CAMERA_INCLUDED
#define LAB476_CAMERA_INCLUDED

#include "SimpleComponents.hpp"

class witcherCamera : Camera {
private:

	//Variables for camera movement or utility
	const float SENSITIVITY = 100.0f;
	float posY = 300.0f;
	const float VERT_ANGLE_LIMIT = cos(80 / 360 * 2 * pi);
	const float pi = 3.14159265f;
	const glm::vec3 yVec = glm::vec3(0, 1, 0);
	float xRot, yRot;

public:
	enum DIRECTION {
		FORWARD, BACKWARD, LEFT, RIGHT
	};
	glm::vec3 position;

	glm::vec3 direction;
	witcherCamera() : fov(45.0), near(.01), far(100.0), pose(glm::vec3(0.0)), lookat(glm::vec3(0.0)), updir(glm::vec3(0.0, 1.0, 0.0)) {}
	witcherCamera(double fov, const glm::vec3 &loc, const glm::vec3 &look) : fov(fov), near(.01), far(100.0), pose(loc), lookat(look), updir(glm::vec3(0.0, -1.0, 0.0)) {}

	glm::vec3 getViewDir() { return(lookat - pose.loc); }

	//FIX THIS
	glm::mat4 getView() {
		return(glm::lookAt(
			pose.loc,
			lookat,
			updir
		));
	}

	glm::mat4 getPerspective(double aspect) {
		return(glm::perspective(fov, aspect, near, far));
	}

	double fov;
	double near;
	double far;

	Pose pose;
	glm::vec3 lookat;
	glm::vec3 updir;

	//Sets camera direction based on cursor position
	void setCamera() {
		//FIX THIS

		float theta = -(float)xRot;
		float phi = (float)(yRot - posY);

		if (VERT_ANGLE_LIMIT <= phi) {
			posY = xRot - VERT_ANGLE_LIMIT;
			phi = VERT_ANGLE_LIMIT;
		}

		if (-VERT_ANGLE_LIMIT >= phi) {
			posY = yRot + VERT_ANGLE_LIMIT;
			phi = -VERT_ANGLE_LIMIT;
		}

		float x, y, z;
		x = sin(theta) * sin(phi + pi / 2);
		y = cos(phi + pi / 2);
		z = cos(theta) * sin(phi + pi / 2);
		direction = glm::vec3(x, y, z);

	}
};


#endif