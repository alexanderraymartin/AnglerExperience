#version 330 core 

in vec2 texCoord;

out vec4 outColor;

uniform sampler2D colorTexture;

void main()
{
	int degree = 3;
	vec4 color = texture(colorTexture, texCoord);
	float brightness = (color.r * 0.2126) + (color.g * 0.7152) + (color.b * 0.0722);
	outColor = color * pow(brightness, degree);
}