// PAIR: vert_passthru
#version 330 core

in vec3 FragPos;
out vec4 color;
uniform sampler2D tex;
uniform sampler2D depthBufTex;
uniform float focusDepth;

uniform float offset[5] = float[](0.0, 1.0, 2.0, 3.0, 4.0);
uniform float weight[5] = float[](0.06136, 0.24477, 0.38774, 0.24477, 0.06136);

const float tolerance = 0.6; // aperture: bigger value for shallower DoF
const float blurMax = 3.0; // max blur amount

void main(){
	vec2 NDC = ((FragPos.xy + 1.0) * .5);
	float depthValue = vec4(texture(depthBufTex, NDC).rgb, 1).r;
	float factor = (depthValue - focusDepth);

	float blur = clamp(factor * tolerance, -blurMax, blurMax);
		
	color = texture(tex, NDC);
	color += vec4(texture(tex, NDC).rgb, 1)*weight[0];

	for (int i=1; i < 5; i ++) {
		color += blur * vec4(texture(tex, NDC + vec2(offset[i], 0.0)/512.0).rgb, 1)*weight[i];
		color += blur * vec4(texture(tex, NDC - vec2(offset[i], 0.0)/512.0).rgb, 1)*weight[i];
	}
}
