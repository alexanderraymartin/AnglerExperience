#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec4 gSpecular;


//uniform sampler2D texture_diffuse;
//uniform sampler2D texture_specular;

uniform vec3 matAmb;
uniform vec3 matDif;
uniform vec3 matSpec;
uniform float matShine;

in vec3 fragNor;
in vec3 fragPos;
in vec2 TextureCoordinates;

//ALL VALUES ARE IN WORLD SPACE
void main()
{
	//pass through the normal and position information
	gNormal = normalize(fragNor);
	gPosition = fragPos;

//If material, use this
	gAlbedo.rgb = matDif;
	gAlbedo.a = length(matAmb);
	gSpecular.rgb = matSpec;
	gSpecular.a = matShine;

//If texture, use this
	//gather the color values from textures, diffuse as an RGB value, and specular as an intensity
	//gAlbedoSpec.rgb = texture(texture_diffuse, TextureCoordinates).rgb;
	//gAlbedoSpec.a = texture(texture_specular, TextureCoordinates).r;
}