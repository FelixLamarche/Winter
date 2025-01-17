#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 lightingColor;

uniform vec3 lightPos;
uniform vec3 lightColor;

layout (std140) uniform Matrices
{
	uniform mat4 projection;
	uniform mat4 view;
};

uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
	// Do not normally inverse here, but for simplicity, we do it here
	// the inverse is needed for the normal matrix to handle non-uniform scaling

	vec3 pos = vec3(model * vec4(aPos, 1.0));
	vec3 normal = normalize(mat3(transpose(inverse(model))) * aNormal);
	vec3 lightDir = normalize(lightPos - pos);

   	// ambient lighting
	float ambientStrength = 0.15;
	vec3 ambient = ambientStrength * lightColor;

	// Diffuse lighting
	float diffuseStrength = 1.0;
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diffuseStrength * diff * lightColor;

	// Specular lighting
	float specularStrength = 0.4;
	int shinyness = 64;
	// normally this would be in view space, with viewPos being (0,0,0)
	vec3 viewDir = normalize(-pos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shinyness);
	vec3 specular = specularStrength * spec * lightColor;

	lightingColor = (ambient + diffuse + specular);
};