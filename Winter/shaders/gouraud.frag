#version 330 core

in vec3 lightingColor;

out vec4 FragColor;

uniform vec3 objectColor;

void main()
{
    FragColor = vec4(lightingColor * objectColor, 1.0);
};