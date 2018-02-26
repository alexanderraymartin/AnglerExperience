#version 330 core

#define ZOOMPOWER (1.0/12.0)
#define INV(_F) (1.0 - (_F))

in vec3 FragPos;
out vec4 FragColor;

uniform sampler2D pixtex;
uniform vec2 resolution;
uniform vec4 mousePos;


void main()
{	
	float aspect = (resolution.x/resolution.y);
	vec2 norm_dev_cord = ((FragPos.xy + 1.0) * .5);

	vec2 ndcMousePos = mousePos.xy/resolution;

	float finalzoom = ZOOMPOWER*INV(mousePos.w) + pow(ZOOMPOWER, 1.5)*mousePos.w;

	vec2 texcoord = mousePos.z*(ndcMousePos - .5*finalzoom) + (norm_dev_cord*(1.0*INV(mousePos.z) + finalzoom*mousePos.z));

	vec4 color = texture(pixtex, texcoord);
	FragColor = color;
}
