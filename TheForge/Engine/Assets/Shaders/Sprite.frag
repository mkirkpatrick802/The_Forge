#version 450

in vec2 uv;
out vec4 color;

uniform sampler2D image;
uniform vec3 sprite_color = vec3(1);

// Multiplied into the final alpha. Defaulted so every existing caller -- SpriteRenderer
// sets its uniforms from a map and never mentions this one -- keeps drawing exactly as
// before. The UI layer uses it to fade whole screens in and out.
uniform float opacity = 1.0;

void main()
{    
    vec4 texture_color = texture(image, uv);
    if (texture_color.a < .1)
        discard;

    color = vec4(sprite_color, 1.0) * texture_color;
    color.a *= opacity;
}
