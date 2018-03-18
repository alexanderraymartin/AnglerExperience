#pragma once
#ifndef __LIGHTS_H_
#define __LIGHTS_H_

#include "../Component.hpp"

class PointLight : public Component {
protected:
	glm::vec3 position;
	glm::vec3 color;
	float radius;
};














#endif
