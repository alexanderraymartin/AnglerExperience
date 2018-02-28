// PAIR: vert_passthru
#version 330 core

in vec3 FragPos;
out vec4 color;
uniform sampler2D tex;
uniform sampler2D depthBufTex;
uniform float focusDepth;

uniform float offset[5] = float[](0.0, 1.0, 2.0, 3.0, 4.0);
uniform float weight[5] = float[](0.06136, 0.24477, 0.38774, 0.24477, 0.06136);

void main(){
	vec2 NDC = ((FragPos.xy + 1.0) * .5);
	float depthValue = vec4(texture(depthBufTex, NDC).rgb, 1).r;

	color = vec4(texture(tex, NDC).rgb, 1)*weight[0];

	for (int i=1; i < 5; i ++) {
		color += abs(depthValue - focusDepth) * vec4(texture(tex, NDC + vec2(0.0, offset[i])/512.0).rgb, 1)*weight[i];
		color += abs(depthValue - focusDepth) * vec4(texture(tex, NDC - vec2(0.0, offset[i])/512.0).rgb, 1)*weight[i];
	}
}
