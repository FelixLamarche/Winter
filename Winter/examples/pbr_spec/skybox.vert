#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
	TexCoords = aPos;

	mat4 rotView = mat4(mat3(view)); // Remove translation from the view matrix
	vec4 clipPos = projection * rotView * vec4(aPos, 1.0);
	gl_Position = clipPos.xyww; // We make sure the depth is 1.0 to render the cubemap at the far plane
}