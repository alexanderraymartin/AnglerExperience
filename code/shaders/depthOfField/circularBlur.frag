// PAIR: vert_passthru
#version 330 core

in vec3 FragPos;
out vec4 color;
uniform sampler2D tex;
uniform sampler2D depthBufTex;
uniform float focusDepth;

const float blurMax = 3.0;  // max blur amount
const float bias = 0.6; //aperture: bigger values gives shallower DoF
 
uniform float weight1[32] = float[](0.0,0.4,   0.15,0.37,   0.29,0.29,   -0.37,0.15,   0.4,0.0,   0.37,-0.15,   0.29,-0.29,   -0.15,-0.37,   0.0,-0.4,
									-0.15,0.37,   -0.29,0.29,   0.37,0.15,    -0.4,0.0,   -0.37,-0.15,   -0.29,-0.29,   0.15,-0.37);
									
uniform float weight2[16] = float[](0.15,0.37,   -0.37,0.15,   0.37,-0.15,   -0.15,-0.37,   -0.15,0.37,   0.37,0.15,   -0.37,-0.15,   0.15,-0.37);
									
uniform float weight3[16] = float[](0.29,0.29,   0.4,0.0,   0.29,-0.29,   0.0,-0.4,   -0.29,0.29,   -0.4,0.0,   -0.29,-0.29,   0.0,0.4);
								
									
									
 
void main()
{
	float aspectratio = 1920.0/1080.0;
	vec2 aspectcorrect = vec2(1.0,aspectratio);

	vec2 NDC = ((FragPos.xy + 1.0) * .5);
	vec4 depth = texture(depthBufTex, NDC);

	float factor = (depth.x - focusDepth);
	 
	vec2 blur = vec2 (clamp(factor * bias, -blurMax, blurMax));


	color = vec4(0.0);
	color += texture(tex, NDC);

	for (int i=0; i < 32 / 2; i+=2) 
	{
		color += texture(tex, NDC + (vec2(weight1[i], weight1[i+1])*aspectcorrect) * blur);
	}
	
	for (int i=0; i < 16 / 2; i+=2) 
	{
		color += texture(tex, NDC + (vec2(weight2[i], weight2[i+1])*aspectcorrect) * blur*0.9);
	}
	
	for (int i=0; i < 16 / 2; i+=2) 
	{
		color += texture(tex, NDC + (vec2(weight3[i], weight3[i+1])*aspectcorrect) * blur*0.7);
		color += texture(tex, NDC + (vec2(weight3[i], weight3[i+1])*aspectcorrect) * blur*0.4);
	}  
				   
	color = color/41.0;
	color.a = 1.0;
}