#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;

out vec3 fragNor;
out vec3 fragView;
out vec3 fragPos;

void main()
{
	gl_Position = P * V * M * vertPos;
	fragNor = (transpose(inverse(M)) * vec4(vertNor, 0.0)).xyz;
	fragView = -(V * M * vertPos).xyz;
	fragPos = (M * vertPos).xyz;
}
