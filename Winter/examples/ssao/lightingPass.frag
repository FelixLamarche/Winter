#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;

struct Light {
    vec3 Position;
    vec3 Color;

    float Constant;
    float Linear;
    float Quadratic;
};
const int NR_LIGHTS = 8;
uniform Light lights[NR_LIGHTS];

void main()
{             
    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float AmbientOcclusion = texture(ssao, TexCoords).r;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 lighting = 0.1 * AmbientOcclusion * Albedo;
    vec3 viewDir = normalize(- FragPos); // In view-space, so viewPos is (0,0,0)
    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // diffuse
        vec3 lightDir = normalize(lights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * lights[i].Color;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = lights[i].Color * spec * Specular;
        // attenuation
        float distance = length(lights[i].Position - FragPos);
        float attenuation = 1.0 / (lights[i].Constant + lights[i].Linear * distance + lights[i].Quadratic * (distance * distance));

        lighting += (diffuse + specular) * attenuation;
    }
    
    FragColor = vec4(lighting, 1.0);
}  