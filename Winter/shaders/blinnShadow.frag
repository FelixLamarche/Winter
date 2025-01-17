#version 330 core
struct Material {
    sampler2D texture_diffuse0;
    sampler2D texture_specular0;
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
	vec3 Normal;
	vec2 TexCoords;
	vec4 FragPosLightSpace;
} fs_in;

out vec4 FragColor;

#define NB_POINT_LIGHTS 4

uniform Material material;
uniform vec3 viewPos;
uniform DirLight dirLight;
uniform sampler2D shadowMap;
uniform SpotLight spotLight;
uniform PointLight pointLights[NB_POINT_LIGHTS];
uniform samplerCube depthCubemap;
uniform float far_planeCube;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float ShadowCalculationDir(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal);
float ShadowCalculationPoint(vec3 fragPos, vec3 lightPos, vec3 normal);

void main()
{
	vec3 normal = normalize(fs_in.Normal);
	vec3 viewDir = normalize(viewPos - fs_in.FragPos);

	vec3 result = vec3(0.0);
	//result += CalcDirLight(dirLight, normal, viewDir);
	for(int i = 0; i < 1; i++)
	{
		result += CalcPointLight(pointLights[i], normal, fs_in.FragPos, viewDir);
	}
	//result += CalcSpotLight(spotLight, normal, fs_in.FragPos, viewDir);

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

	// Shadows
	float shadow = ShadowCalculationDir(fs_in.FragPosLightSpace, lightDir, normal);
	vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

	return lighting;
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

	// Shadow
	float shadow = ShadowCalculationPoint(fs_in.FragPos, light.position, normal);
	vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * attenuation;

	return lighting;
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

float ShadowCalculationDir(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;

    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
	// check whether current frag pos is in shadow
	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	if(projCoords.z > 1.0)
	{
	    shadow = 0.0;
	}
	else
	{
		for(int x = -1; x <= 1; x++)
		{
			for(int y = -1; y <= 1; y++)
			{
				float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
				shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
			}
		}
		shadow /= 9.0;
	}

    return shadow;
}  

float ShadowCalculationPoint(vec3 fragPos, vec3 lightPos, vec3 normal)
{
	vec3 sampleOffsetDirections[20] = vec3[]
	(
	   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
	   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
	   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
	   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
	   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
	);   

	// get vector between fragment position and light position
    vec3 fragToLight = fragPos - lightPos;
	vec3 lightDir = normalize(lightPos - fragPos);
    float closestDepth = texture(depthCubemap, fragToLight).r;
    closestDepth *= far_planeCube;
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // now test for shadows
    float shadow = 0.0;
	float bias   = 0.15;
	int samples  = 20;
	float viewDistance = length(viewPos - fragPos);
	float diskRadius = (1.0 + (viewDistance / far_planeCube)) / 25.0;  
	for(int i = 0; i < samples; ++i)
	{
		// use the light to fragment vector to sample from the depth map    
		float closestDepth = texture(depthCubemap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
		// it is currently in linear range between [0,1]. Re-transform back to original value
		closestDepth *= far_planeCube;   // undo mapping [0;1]
		if(currentDepth - bias > closestDepth)
			shadow += 1.0;
	}
	shadow /= float(samples);

    return shadow;
}  