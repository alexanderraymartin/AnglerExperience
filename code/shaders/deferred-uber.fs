#version 330 core

#define MAX_POINT_LIGHTS 16
#define BACKGROUND vec3(.18, .20, .22)
#define EPS 1e-7

#define INV(_F) (1.0 - _F)

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gSpecular;

struct PointLights {
  vec3 positions[MAX_POINT_LIGHTS];
  vec3 colors[MAX_POINT_LIGHTS];
};
uniform PointLights pointLights;
uniform int numLights;

uniform vec3 sunCol;
uniform vec3 sunDir;

uniform vec3 viewPos;

uniform sampler2D caustic;
uniform sampler2D shadowMap;

uniform mat4 causticMatrix;
uniform mat4 shadowMatrix;

uniform float levelProgress;

void main()
{             
  // retrieve data from G-buffer
  vec3 FragPos = texture(gPosition, TexCoords).rgb;
  vec3 Normal = texture(gNormal, TexCoords).rgb;
  vec3 Albedo = texture(gAlbedo, TexCoords).rgb;
  float Ambient = texture(gAlbedo, TexCoords).a;
  vec3 SpecColor = texture(gSpecular, TexCoords).rgb;
  float shine = texture(gSpecular, TexCoords).a;

  float BackMask = 1.0 - step(EPS, length(Normal));
  vec3 sunVec = -sunDir;
  
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 H = normalize(sunVec+viewDir);
  vec3 lighting = Albedo * Ambient; // hard-coded ambient component

  lighting += Albedo * max(Albedo*sunCol*dot(Normal, sunVec),0.0) + max(SpecColor*sunCol*pow(dot(Normal, H),shine)*step(0.0, dot(sunVec, Normal)), 0.0);

  int lightbound = min(numLights, MAX_POINT_LIGHTS);
  for(int i = 0; i < lightbound; ++i)
  {
    vec3 lightDir = normalize(pointLights.positions[i] - FragPos);
    vec3 H = normalize(pointLights.positions[i]+viewDir);
    float facingmask = step(0.0, dot(lightDir, Normal));
    float attenuation = 1.0 / pow(length(pointLights.positions[i] - FragPos), 2.0);

    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * pointLights.colors[i];
    vec3 specular = max(SpecColor*pointLights.colors[i]*pow(dot(Normal, H), shine)*facingmask, 0.0);

    lighting += (diffuse + specular) * attenuation;
  }

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
/*
    // create a ball of fog
    vec3 line = normalize(FragPos - viewPos);
    vec3 orig = FragPos;
    vec3 center = vec3(0.5, 3, 7);
    float radius = 0.5;

    float firstpart = -(dot(line, (orig - center)));
    float stuffundersqrt = pow(firstpart, 2) - pow(length(orig - center), 2) + radius*radius;
*/
    /* If it intersects the circle, do fog */
/*
    if (stuffundersqrt > 0) {
        float secondpart = sqrt(stuffundersqrt);
        float dist = distance(firstpart + secondpart, firstpart - secondpart);
        lighting = mix(lighting, vec3(0.7), dist / radius / 3.0);
    }
*/

  FragColor = vec4(lighting, 1.0);
}
