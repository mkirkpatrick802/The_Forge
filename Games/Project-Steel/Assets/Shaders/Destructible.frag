#version 330 core
in vec2 uv;
out vec4 color;

uniform sampler2D image;
uniform vec2 size;
uniform float health; // Percentage of pixels to disappear (0.0 - 1.0)

void main() 
{
    // Convert UVs to sprite grid space for pixel-perfect alignment
    vec2 pixelCoord = floor(uv * size) / size;

    // Compute the distance from the center of the sprite
    vec2 center = vec2(0.5, 0.5);
    float distanceToCenter = length(pixelCoord - center);

    // Normalize distance (max distance is sqrt(0.5^2 + 0.5^2) ≈ 0.707)
    float maxDistance = length(vec2(0.5, 0.5));
    float normalizedDistance = distanceToCenter / maxDistance;

    float disappearChance = 1.0 - health; // Flip so 0 = all disappear, 1 = none disappear
    
    // Generate a random value per pixel (consistent per pixel)
    float randomValue = fract(sin(dot(pixelCoord, vec2(12.9898, 78.233))) * 43758.5453);

    // Adjust random threshold so that outer pixels disappear first
    float threshold = mix(0.0, 1.0, normalizedDistance); // Higher for outer pixels

    // Pixels disappear if their random value is below threshold * u_Percent
    if (randomValue < threshold * disappearChance) 
    {
        discard;
    } 
    else 
    {
        color = texture(image, pixelCoord);
    }
}
