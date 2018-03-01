#version 330 core

layout (location = 0) in vec3 vertPos;

uniform mat4 M;
uniform mat4 depthV;
uniform mat4 depthP;

void main()
{
  gl_Position = depthP * depthV * M * vec4(vertPos, 1.0);
}
