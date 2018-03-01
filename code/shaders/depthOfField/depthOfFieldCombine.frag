// PAIR: vert_passthru
#version 330 core

in vec3 FragPos;
out vec4 color;
uniform sampler2D tex;
uniform sampler2D blurredTex
uniform sampler2D depthBufTex;

float linearizeDepth(in float depth)
{
	float zNear = 0.1;
	float zFar  = 100.0;
	return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main(){
	vec2 NDC = ((FragPos.xy + 1.0) * .5);
	float depthValue = linearizeDepth(vec4(texture(depthBufTex, NDC).rgb, 1).r);
		
	color = mix(tex, blurredTex, depthValue);
	
}
