#version 450
layout(location = 0) in vec2 position;

uniform mat4 projection;
void main()
{
    gl_Position = projection * vec4(position, 0, 1);
}