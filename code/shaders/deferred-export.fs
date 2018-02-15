#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;


//uniform sampler2D texture_diffuse;
//uniform sampler2D texture_specular;

uniform vec3 uMatAmb;
uniform vec3 uMatDif;
uniform vec3 uMatSpec;
uniform float uShine;

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
	gAlbedoSpec.rgb = uMatDif;
	gAlbedoSpec.a = uShine;

	//gl_FragData[0] = gPosition;
	//gl_FragData[1] = gNormal;
	//gl_FragData[2] = gAlbedoSpec;

//If texture, use this
	//gather the color values from textures, diffuse as an RGB value, and specular as an intensity
	//gAlbedoSpec.rgb = texture(texture_diffuse, TextureCoordinates).rgb;
	//gAlbedoSpec.a = texture(texture_specular, TextureCoordinates).r;
}
