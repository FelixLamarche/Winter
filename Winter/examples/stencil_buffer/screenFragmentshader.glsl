#version 330 core
out vec4 FragColor;

uniform vec3 color;

void main()
{
	FragColor = vec4(vec3(gl_FragDepth), 1.0);
}