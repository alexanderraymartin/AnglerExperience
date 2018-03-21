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

uniform vec3 color;
uniform vec3 lightDir;

uniform vec3 viewPos;

uniform sampler2D caustic;
uniform sampler2D shadowMap;

uniform mat4 causticMatrix;
uniform mat4 shadowMatrix;

uniform float levelProgress;

void main()
{
	vec2 TexCoords = gl_FragCoord.xy / vec2(textureSize(gPosition, 0));
  // retrieve data from G-buffer
  vec3 FragPos = texture(gPosition, TexCoords).rgb;
  vec3 Normal = texture(gNormal, TexCoords).rgb;
  vec3 Albedo = texture(gAlbedo, TexCoords).rgb;
  float Ambient = texture(gAlbedo, TexCoords).a;
  vec3 SpecColor = texture(gSpecular, TexCoords).rgb;
  float shine = texture(gSpecular, TexCoords).a;

  float BackMask = 1.0 - step(EPS, length(Normal));
  vec3 sunVec = -lightDir;
  
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 H = normalize(sunVec+viewDir);
  //vec3 lighting = Albedo * Ambient; // hard-coded ambient component

  vec3 lighting = Albedo * max(Albedo*color*dot(Normal, sunVec),0.0) + max(SpecColor*color*pow(dot(Normal, H),shine)*step(0.0, dot(sunVec, Normal)), 0.0);


  vec4 shadowCoord = shadowMatrix * texture(gPosition, TexCoords);
  vec4 causticCoord = causticMatrix * texture(gPosition, TexCoords);

  vec4 caustColor = texture(caustic, causticCoord.xy) * INV(BackMask);

  float visibility = 1.0;
  float bias = 0.005;

  if ( texture( shadowMap, shadowCoord.xy ).x  <  shadowCoord.z - bias){
    visibility = 0.25;
  }

  lighting = mix(lighting, lighting*caustColor.rgb, caustColor.a*.8) * INV(BackMask);

  lighting = mix(lighting, BACKGROUND, BackMask) * visibility;

    // exponential fog
    float fogDensity = .051;
    float linearDepth = FragPos.z-4;
    float fog = 1.0 - clamp(exp(-fogDensity*linearDepth), 0.0, 1.0); 
    vec3 fogColor = vec3(0.2, 0.2, 0.3);
    lighting = mix (lighting, fogColor, fog + BackMask);

  FragColor = vec4(lighting, 1.0);
}
