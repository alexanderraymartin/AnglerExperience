#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light {
    vec3 Position;
    vec3 Color;
};
const int NR_LIGHTS = 32;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

void main()
{             
    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 lighting = Albedo * 0.1; // hard-coded ambient component
    vec3 viewDir = normalize(viewPos - FragPos);
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        //vec3 lightDir = vec3(1.0, 1.0, 0.0);
        float attenuation = 1.0 / (1.0 + 0.02 * pow(length(lights[i].Position - FragPos), 3));
        vec3 lightDir = normalize(lights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * lights[i].Color;
        vec3 shineLight = diffuse.r > 0.0 || diffuse.g > 0.0 || diffuse.b > 0.0 ? pow(max(dot(reflect(-lightDir, Normal), normalize(viewPos - FragPos)), 0.0), Specular) * Albedo : vec3(0.0, 0.0, 0.0);
        lighting += (diffuse + shineLight) * attenuation;
    }
    
    FragColor = vec4(lighting, 1.0);
}  