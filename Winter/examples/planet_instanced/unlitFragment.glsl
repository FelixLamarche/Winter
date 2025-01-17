#version 330 core
struct Material {
    sampler2D texture_diffuse0;
    sampler2D texture_specular0;
    float shininess;
}; 

in vec2 TexCoords;

out vec4 FragColor;

uniform Material material;

void main()
{
    FragColor = texture(material.texture_diffuse0, TexCoords);
};
