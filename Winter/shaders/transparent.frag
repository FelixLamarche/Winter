#version 330 core
struct Material {
	sampler2D texture_diffuse0;
	float shininess;
};

out vec4 FragColor;

in vec2 TexCoords;

uniform Material material;

void main()
{
	vec4 texColor = texture(material.texture_diffuse0, TexCoords);
	if(texColor.a < 0.1)
		discard;
	FragColor = texColor;
}