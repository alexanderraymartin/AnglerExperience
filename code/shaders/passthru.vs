#version  330 core
layout(location = 0) in vec4 vertPos;

uniform vec2 resolution;

void main()
{
	gl_Position = vertPos;
}
