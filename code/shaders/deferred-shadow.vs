#version 330 core

layout (location = 0) in vec3 vertPos;

uniform mat4 M;
uniform mat4 depthMatrix;

void main()
{
  gl_Position = depthMatrix * M * vec4(vertPos, 1.0);
}
