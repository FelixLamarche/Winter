#version 330 core
in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 viewPos;
uniform samplerCube skybox;

void main()
{
	vec3 norm = normalize(Normal);

	float ratio = 1.00 / 1.52;
	vec3 I = normalize(FragPos - viewPos);
	// Refraction
	vec3 R = refract(I, norm, ratio);
	vec3 refractColor = texture(skybox, R).rgb;
	// Reflection
	vec3 R2 = reflect(I, norm);
	vec3 reflectColor = texture(skybox, R2).rgb;
	FragColor = vec4(reflectColor, 1.0);
};
