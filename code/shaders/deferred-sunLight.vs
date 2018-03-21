#version 330 core
layout(location = 0) in vec3 vertPos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;


void main()
{
	gl_Position = vec4(vertPos, 1.0);
}