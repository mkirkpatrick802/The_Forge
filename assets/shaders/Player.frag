#version 450

in vec2 uv;

out vec4 color;

uniform sampler2D image;
uniform vec3 sprite_color = vec3(1);

uniform float health = 1;
uniform vec2 texture_size = vec2(16);
uniform float outline_scale = 2;

void main()
{    
    vec4 texture_color = texture(image, uv);

    // Glow color based on health
    vec3 glowColor = vec3(1.0, 0.0, 0.0); // Red glow

    // Determine if we are at an edge
    float x_offset = (1.f / texture_size.x) * outline_scale;
    float y_offset = (1.f / texture_size.y) * outline_scale;

    vec4 neighbor1 = texture(image, uv + vec2(x_offset, 0)); // Right neighbor
    vec4 neighbor2 = texture(image, uv + vec2(-x_offset, 0)); // Left neighbor
    vec4 neighbor3 = texture(image, uv + vec2(0, y_offset)); // Top neighbor
    vec4 neighbor4 = texture(image, uv + vec2(0, -y_offset)); // Bottom neighbor
   
    float alpha = texture_color.a;
    float minNeighborAlpha = max(max(neighbor1.a, neighbor2.a), max(neighbor3.a, neighbor4.a));

    // Check if this is an edge pixel
    if (alpha < minNeighborAlpha)
    {
        if(health == 1)
            color = vec4(vec3(0.216, 0.541, 0.267), .5);
        else
            color = vec4(glowColor * health, .75);
    }
    else
    {
        // Regular sprite rendering
        color = vec4(sprite_color, 1.0) * texture_color;
    }
} 