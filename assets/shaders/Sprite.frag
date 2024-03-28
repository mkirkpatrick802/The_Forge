#version 450

in vec2 uv;

out vec4 color;

uniform sampler2D image;
uniform vec3 sprite_color = vec3(1);

void main()
{    
    color = vec4(sprite_color, 1.0) * texture(image, uv);
} 