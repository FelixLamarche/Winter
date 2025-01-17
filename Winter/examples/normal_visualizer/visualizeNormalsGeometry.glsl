#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

out vec3 Normal;

const float MAGNITUDE = 0.4;

layout (std140) uniform Matrices
{
	uniform mat4 projection;
	uniform mat4 view;
};
void GenerateLine(int index)
{
    vec3 normalProj = vec3(projection * vec4(gs_in[index].normal, 1.0));
    gl_Position = projection * gl_in[index].gl_Position;
    Normal = normalProj;
    EmitVertex();
    gl_Position = projection * (gl_in[index].gl_Position + 
                                vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
    Normal = normalProj;
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal
}  