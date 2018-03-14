#pragma once
#ifndef MOUSEPROCESSING_H_
#define MOUSEPROCESSING_H_

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "components/Camera.h"

using namespace glm;

namespace MouseProcessing {
  vec3 getWoldSpace(double mouseX, double mouseY, GLFWwindow* window, Camera* camera);
};

#endif
