// PAIR: vert_passthru
#version 330 core 

in vec3 FragPos;

out vec4 outColor;

uniform sampler2D tex;

void main()
{
	vec2 NDC = ((FragPos.xy + 1.0) * 0.5);
	float degree = 1.5;
	vec4 color = texture(tex, NDC);
	float brightness = (color.r * 0.2126) + (color.g * 0.7152) + (color.b * 0.0722);
	outColor = color * pow(brightness, degree);
}