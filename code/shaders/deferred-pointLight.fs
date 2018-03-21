#version 330 core

#define MAX_POINT_LIGHTS 16
#define BACKGROUND vec3(.18, .20, .22)
#define EPS 1e-7

#define INV(_F) (1.0 - _F)

out vec4 FragColor;


uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSpecular;

uniform vec3 lightPos;
uniform vec3 color;
uniform vec3 viewPos;

uniform float levelProgress;

void main()
{             
	vec2 TexCoords = gl_FragCoord.xy / vec2(textureSize(gPosition, 0));
  // retrieve data from G-buffer
  vec3 FragPos = texture(gPosition, TexCoords).rgb;
  vec3 Normal = texture(gNormal, TexCoords).rgb;
  vec3 Albedo = texture(gAlbedo, TexCoords).rgb;
  vec3 SpecColor = texture(gSpecular, TexCoords).rgb;
  float shine = texture(gSpecular, TexCoords).a;

  float BackMask = 1.0 - step(EPS, length(Normal));
  
  vec3 viewDir = normalize(viewPos - FragPos);

  vec3 lightDir = normalize(lightPos - FragPos);
  vec3 H = normalize(lightPos + viewDir);
  float facingmask = step(0.0, dot(lightDir, Normal));
  float attenuation = 1.0 / pow(length(lightPos - FragPos), 2.0);

  vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * color;
  vec3 specular = max(SpecColor*color*pow(dot(Normal, H), shine)*facingmask, 0.0);

  vec3 lighting = (diffuse + specular) * attenuation;
/*
  // exponential fog
  float fogDensity = .051;
  float linearDepth = FragPos.z-4;
  float fog = 1.0 - clamp(exp(-fogDensity*linearDepth), 0.0, 1.0); 
  vec3 fogColor = vec3(0.2, 0.2, 0.3);
  lighting = mix (lighting, fogColor, fog + BackMask);
*/
  //FragColor = vec4(1.0);
  FragColor = vec4(lighting, 1.0);
}
