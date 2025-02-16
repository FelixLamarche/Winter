#version 330 core
  
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 WorldPos;
out vec3 Normal;
out vec2 TexCoords;

layout (std140) uniform Matrices
{
	uniform mat4 projection;
	uniform mat4 view;
};

uniform mat4 model;
uniform mat3 normalMatrix;
uniform vec2 texScale;

void main()
{
   WorldPos = vec3(model * vec4(aPos, 1.0));
   Normal = normalMatrix * aNormal;
   TexCoords = aTexCoords * texScale;

   gl_Position = projection * view * model * vec4(aPos, 1.0);
};