#version 330 core

in vec3 FragPos;
out vec4 FragColor;

uniform sampler2D pixtex;

void main()
{	
	vec2 norm_dev_cord = ((FragPos.xy + 1.0) * .5);
	vec4 color = texture(pixtex, norm_dev_cord);
	FragColor = color;
}
