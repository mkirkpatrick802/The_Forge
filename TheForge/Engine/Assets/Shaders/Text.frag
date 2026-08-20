#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D text;
uniform vec3 textColor;

// See Sprite.frag: defaulted so the existing TextRenderer, which never sets it, is
// unaffected.
uniform float opacity = 1.0;

void main() 
{
    float alpha = texture(text, TexCoords).r;
    FragColor = vec4(textColor, alpha * opacity);
}
