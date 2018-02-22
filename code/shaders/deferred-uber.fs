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

uniform mat4 depthV;
uniform mat4 depthP;
uniform mat4 depthB;

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

    vec4 depthCoord = depthB * depthP * depthV * texture(gPosition, TexCoords);
    vec4 caustColor = texture(caustic, depthCoord.xy) * INV(BackMask);

    //lighting = mix(lighting, lighting*caustColor.rgb, caustColor.a) * INV(BackMask);

    lighting = mix(lighting, BACKGROUND, BackMask);
    
    FragColor = vec4(lighting, 1.0);
}
