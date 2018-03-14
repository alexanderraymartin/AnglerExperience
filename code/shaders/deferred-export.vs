#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in vec3 vertPos2;
layout(location = 4) in vec3 vertNor2;
layout(location = 5) in vec2 vertTex2;

uniform mat4 P;
uniform mat4 M;
uniform mat4 V;
uniform float interp;


out vec3 fragNor;
out vec2 TextureCoordinates;
out vec3 fragPos;

void main()
{
	vec3 pos = vertPos * (1 - interp) + vertPos2 * interp;
	vec3 nor = vertNor * (1 - interp) + vertNor2 * interp;

	vec3 position = pos;
	fragNor = normalize(transpose(inverse(mat3(M))) * nor);
	fragPos = (M * vec4(position, 1.0)).xyz;
	gl_Position = P * V * M * vec4(position, 1.0);
	TextureCoordinates = vertTex;
}
