#version 450

in vec2 uv;

out vec4 color;

uniform sampler2D image;
uniform vec3 spriteColor = vec3(0);

void main()
{    
    color = vec4(spriteColor, 1.0) * texture(image, uv);
} 