#version 450

layout (location = 0) in vec4 vertex; // Position & Texture Coordinates

out vec2 TexCoords;
uniform mat4 projection;

void main() 
{
    TexCoords = vertex.zw;
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
}
