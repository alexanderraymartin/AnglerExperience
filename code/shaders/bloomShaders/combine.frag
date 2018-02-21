#version 330 core 

in vec2 texCoord;

out vec4 outColor;

uniform sampler2D colorTexture;
uniform sampler2D brightTexture;

void main()
{
	int intensity = 1;
	vec4 color = texture(colorTexture, texCoord);
	vec4 brightColor = texture(brightTexture, texCoord);
	
	outColor = color + brightColor * intensity;
}