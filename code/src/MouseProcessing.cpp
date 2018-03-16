#include "MouseProcessing.hpp"

vec4 getClipRay(double mouseX, double mouseY, int w_width, int w_height) {
  float x = 2.0 * mouseX / w_width - 1;
  float y = 1 - 2.0 * mouseY / w_height;

  return vec4(x, y, -1.0f, 1.0f);
}

vec4 getEyeCoords(vec4 clipRay, mat4 perspective) {
  vec4 eyeCoords = inverse(perspective) * clipRay;
  return vec4(eyeCoords.x, eyeCoords.y, -1.0f, 0.0f);
}

vec4 getWorldRay(vec4 eyeCoords, mat4 view) {
  return inverse(view) * eyeCoords;
}

vec3 getScaledRay(vec4 worldRay, vec3 cameraPos, float depth) {
  vec3 rayDirection = normalize(vec3(worldRay));

  float scale = dot((vec3(0,0,depth) - cameraPos), vec3(0, 0, -1)) / (dot(rayDirection, vec3(0, 0, -1)));

  return rayDirection * scale + cameraPos;
}

vec3 MouseProcessing::getWoldSpace(double mouseX, double mouseY, GLFWwindow* window, Camera* camera) {
  int w_width, w_height;
  glfwGetWindowSize(window, &w_width, &w_height);

  float aspect = w_width/float(w_height);

  vec4 clipRay = getClipRay(mouseX, mouseY, w_width, w_height);
  vec4 eyeCoords = getEyeCoords(clipRay, camera->getPerspective(aspect));
  vec4 worldRay = getWorldRay(eyeCoords, camera->getView());

  return getScaledRay(worldRay, camera->getLocation(), 3.0f);
}

