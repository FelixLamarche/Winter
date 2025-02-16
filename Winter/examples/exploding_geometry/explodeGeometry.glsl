#version 330 core

layout (triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} gs_in[];

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform float time;

vec4 explode(vec4 position, vec3 normal) 
{
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(time) + 1.0) / 2.0) * magnitude; 
    return position + vec4(direction, 0.0);
}

vec3 getNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
    return normalize(cross(a, b));
}

void main()
{
    vec3 normal = getNormal();

    gl_Position = explode(gl_in[0].gl_Position, normal);
    FragPos = gs_in[0].FragPos;
    Normal = gs_in[0].Normal;
    TexCoords = gs_in[0].TexCoords;
    EmitVertex();
    gl_Position = explode(gl_in[1].gl_Position, normal);
    FragPos = gs_in[1].FragPos;
    Normal = gs_in[1].Normal;
    TexCoords = gs_in[1].TexCoords;
    EmitVertex();
    gl_Position = explode(gl_in[2].gl_Position, normal);
    FragPos = gs_in[2].FragPos;
    Normal = gs_in[2].Normal;
    TexCoords = gs_in[2].TexCoords;
    EmitVertex();
    EndPrimitive();
}