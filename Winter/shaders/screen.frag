#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec3 res = vec3(texture(screenTexture, TexCoords.st));
    FragColor = vec4(res.xxx, 1.0);
}