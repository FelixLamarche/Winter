#version 330 core
out vec4 FragColor;
out vec4 BrightColor;

uniform vec3 color;

void main()
{
	FragColor = vec4(color, 1.0);
	BrightColor = FragColor;
}