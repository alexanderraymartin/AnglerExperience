// PAIR: vert_passthru
#version 330 core

in vec3 FragPos;
out vec4 color;
uniform sampler2D tex;

const int kernelSize = 7;
uniform float offset[kernelSize] = float[](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0);
uniform float weight[kernelSize] = float[](0.197448, 0.174697, 0.120999, 0.065602, 0.02784, 0.009246, 0.002403);

uniform vec2 resolution;

void main(){
	vec2 NDC = ((FragPos.xy + 1.0) * .5);
	color = vec4(texture(tex, NDC).rgb, 1)*weight[0];

	for (int i=1; i < kernelSize; i ++) {
		color += vec4(texture(tex, NDC + vec2(offset[i], 0.0)/resolution).rgb, 1)*weight[i];
		color += vec4(texture(tex, NDC - vec2(offset[i], 0.0)/resolution).rgb, 1)*weight[i];
	}
}
