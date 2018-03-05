// PAIR: vert_passthru
#version 330 core

in vec3 FragPos;
out vec4 color;
uniform sampler2D tex;
uniform sampler2D blurredTex;
uniform sampler2D depthBufTex;

float linearizeDepth(in float depth)
{
	float zNear = 0.01;
	float zFar  = 100.0;
	return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main(){
	vec2 NDC = ((FragPos.xy + 1.0) * .5);
	float depthValue = vec4(texture(depthBufTex, NDC).rgb, 1).r;
	color = mix(texture(tex, NDC), texture(blurredTex, NDC), linearizeDepth(depthValue));
	
	///////////////////////////////////////////
	// TODO remove after this testing works	
	//float depth = texture(depthBufTex, NDC).r;
	//float linDepth = linearizeDepth(depth);
	//color.r = depth;
	//color.g = linDepth;
	//color.b = sin(linDepth * 3.1415 * 2.0);
	//color.a = 1.0;
	///////////////////////////////////////////
}
