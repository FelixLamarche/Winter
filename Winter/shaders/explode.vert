#version 330 core
  
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

layout (std140) uniform Matrices
{
	uniform mat4 projection;
	uniform mat4 view;
};

uniform mat4 model;

void main()
{
   gl_Position = projection * view * model * vec4(aPos, 1.0);
   vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
   
   // Do not normally inverse here, but for simplicity, we do it here
   // the inverse is needed for the normal matrix to handle non-uniform scaling
   vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;

   vs_out.TexCoords = aTexCoords;
};