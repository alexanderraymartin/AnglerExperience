// PAIR: vert_passthru
#version 330 core 

in vec3 FragPos;

out vec4 outColor;

uniform sampler2D tex;
uniform sampler2D brightTexture;

void main()
{
	vec2 NDC = ((FragPos.xy + 1.0) * .5);
	int intensity = 1;
	vec4 color = texture(tex, NDC);
	vec4 brightColor = texture(brightTexture, NDC);
	outColor = color + brightColor * intensity;
}