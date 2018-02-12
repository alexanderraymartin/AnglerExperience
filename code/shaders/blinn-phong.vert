#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec4 vertPos2;
layout(location = 3) in vec3 vertNor2;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform float interp;

out vec3 fragNor;
out vec3 fragView;
out vec3 fragPos;

void main()
{
	vec4 pos = vertPos * (1 - interp) + vertPos2 * interp;
	vec3 nor = vertNor * (1 - interp) + vertNor2 * interp;

	gl_Position = P * V * M * pos;
	fragNor = (transpose(inverse(M)) * vec4(nor, 0.0)).xyz;
	fragView = -(V * M * pos).xyz;
	fragPos = (M * pos).xyz;

}
