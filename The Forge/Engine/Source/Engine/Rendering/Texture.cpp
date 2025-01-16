#include "Texture.h"
#include "TextureLoader.h"

Engine::Texture::Texture(): _ID(0), _size(), _type()
{
    
}

Engine::Texture::Texture(const TextureType type, const String& filepath)
{
    _ID = LoadTexture(filepath.c_str(), _size);
    _filepath = filepath;
    _type = type;
}

Engine::Texture::Texture(TextureType type, Vector2D size)
{
    
}
