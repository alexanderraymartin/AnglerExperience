// PAIR: vert_passthru
#version 330 core 

in vec3 FragPos;

out vec4 outColor;

uniform sampler2D colorTexture;

void main()
{
	vec2 NDC = ((FragPos.xy + 1.0) * .5);
	int degree = 3;
	vec4 color = texture(colorTexture, NDC);
	float brightness = (color.r * 0.2126) + (color.g * 0.7152) + (color.b * 0.0722);
	outColor = color * pow(brightness, degree);
}