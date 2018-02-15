#version 330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout (location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 M;
uniform mat4 V;


out vec3 fragNor;
out vec2 TextureCoordinates;
out vec3 fragPos;

void main()
{
	vec3 position = vertPos;
	fragNor = normalize(transpose(inverse(mat3(M))) * vertNor);
	fragPos = (M * vec4(position, 1.0)).xyz;
	gl_Position = P * V * M * vec4(position, 1.0);
	TextureCoordinates = vertTex;
}
