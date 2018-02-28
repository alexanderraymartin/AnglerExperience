// PAIR: vert_passthru
#version 330 core

in vec3 FragPos;
out vec4 color;
uniform sampler2D tex;

uniform float offset[7] = float[](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
uniform float weight[7] = float[](0.00598, 0.060626, 0.241843, 0.383103, 0.241843, 0.060626, 0.00598);

void main(){
	vec2 NDC = ((FragPos.xy + 1.0) * .5);
	color = vec4(texture(tex, NDC).rgb, 1)*weight[0];

	for (int i=1; i < 7; i ++) {
		color += vec4(texture(tex, NDC + vec2(offset[i], 0.0)/512.0).rgb, 1)*weight[i];
		color += vec4(texture(tex, NDC - vec2(offset[i], 0.0)/512.0).rgb, 1)*weight[i];
	}
}
