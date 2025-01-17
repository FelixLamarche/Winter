#version 330 core
struct Material {
    sampler2D texture_diffuse0;
    sampler2D texture_specular0;
	sampler2D texture_normal;
    float shininess;
}; 

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight {
	vec3 position;
	
	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct SpotLight {
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;
	
	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in VS_OUT{
	vec3 FragPos;
	vec2 TexCoords;
	vec3 Normal;
	mat3 TBN;
} fs_in;

out vec4 FragColor;

#define NB_POINT_LIGHTS 4

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform SpotLight spotLight;
uniform PointLight pointLights[NB_POINT_LIGHTS];

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 normal = normalize(fs_in.Normal);

	normal = texture(material.texture_normal, fs_in.TexCoords).rgb; // From [0, 1] 
	normal = normal * 2.0 - 1.0; // To [-1, 1]
	normal = normalize(fs_in.TBN * normal);

	vec3 viewDir = normalize(viewPos - fs_in.FragPos);

	vec3 result = CalcDirLight(dirLight, normal, viewDir);
	for(int i = 0; i < NB_POINT_LIGHTS; i++)
	{
		result += CalcPointLight(pointLights[i], normal, fs_in.FragPos, viewDir);
	}
	result += CalcSpotLight(spotLight, normal, fs_in.FragPos, viewDir);

    FragColor = vec4(result, 1.0);
};

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);

	// ambient lighting
	vec3 ambient = light.ambient * texture(material.texture_diffuse0, fs_in.TexCoords).rgb;

	// Diffuse lighting
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse0, fs_in.TexCoords).rgb;

	// Specular lighting
	vec3 halfwaydir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwaydir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.texture_specular0, fs_in.TexCoords).rgb;

	return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// Ambient
	vec3 ambient = light.ambient * texture(material.texture_diffuse0, fs_in.TexCoords).rgb;

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse0, fs_in.TexCoords).rgb;

	// Specular
	vec3 halfwaydir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwaydir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.texture_specular0, fs_in.TexCoords).rgb;

	return (ambient + diffuse + specular) * attenuation;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	float theta = dot(lightDir, normalize(-light.direction));

	if(theta < light.outerCutOff)
	{
		return vec3(0.0);
	}
	
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	
	float distance = length(light.position - fragPos);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	// ambient lighting
	vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse0, fs_in.TexCoords));

	// Diffuse lighting
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture(material.texture_diffuse0, fs_in.TexCoords).rgb;
	diffuse *= intensity;

	// Specular lighting
	vec3 halfwaydir = normalize(lightDir + viewDir);
	float spec = pow(max(dot(normal, halfwaydir), 0.0), material.shininess);
	vec3 specular = light.specular * spec * texture(material.texture_specular0, fs_in.TexCoords).rgb;
	specular *= intensity;

	return (ambient + diffuse + specular) * attenuation;
}
