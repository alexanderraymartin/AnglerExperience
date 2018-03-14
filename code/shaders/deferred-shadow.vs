#version 330 core

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertPos2;

uniform mat4 M;
uniform mat4 depthMatrix;
uniform float interp;

void main()
{
  vec3 pos = vertPos * (1 - interp) + vertPos2 * interp;
  gl_Position = depthMatrix * M * vec4(pos, 1.0);
}
