#version 330 core 
#define EPS 1E-7

in vec3 fragNor;
in vec3 fragView;
in vec3 fragPos;

uniform vec3 sunDir;
uniform vec3 sunCol; 

uniform vec3 matAmb;
uniform vec3 matDif;
uniform vec3 matSpec;
uniform float shine;

out vec4 color;

void main()
{
	vec3 nfragNor = normalize(fragNor);

	vec3 fragLum = matAmb; //Ambient done outside of loop

	vec3 sunVec = -sunDir;
	vec3 H = normalize(sunVec+fragView);
	float facingmask = step(0.0, dot(sunVec, nfragNor));

	fragLum += 
		(matDif)
		* 
		max(matDif*sunCol*dot(nfragNor, sunVec),0.0) // Diffuse 
		+
		max(matSpec*sunCol*pow(dot(nfragNor, H),shine)*facingmask, 0.0); // Specular

	
	vec3 col = fragLum;
	color = vec4(col, 1.0);
}
