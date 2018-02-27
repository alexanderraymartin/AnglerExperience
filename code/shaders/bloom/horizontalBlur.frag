// PAIR: vert_passthru
#version 330 core

in vec3 FragPos;
out vec4 color;
uniform sampler2D brightTexture;

uniform float offset[5] = float[](0.0, 1.0, 2.0, 3.0, 4.0 );
uniform float weight[5] = float[](0.2270270270, 0.1945945946, 0.1216216216, 
											0.0540540541, 0.0162162162 );

void main(){
	vec2 NDC = ((FragPos.xy + 1.0) * .5);
	color = vec4(texture(brightTexture, NDC ).rgb, 1)*weight[0];

	for (int i=1; i < 5; i ++) {
			color += vec4(texture(brightTexture, NDC + vec2(offset[i], 0.0)/512.0).rgb, 1)*weight[i];
			color += vec4(texture(brightTexture, NDC - vec2(offset[i], 0.0)/512.0).rgb, 1)*weight[i];
	}
}
