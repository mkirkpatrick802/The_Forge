#include "Texture.h"
#include "TextureLoader.h"

Engine::Texture::Texture(const GLuint ID, const String& filepath, const TextureType type, const Vector2D size)
{
    _ID = ID;
    _filepath = filepath;
    _type = type;
    _size = size;
}

Engine::Texture::Texture(const GLuint ID, const TextureType type, const Vector2D size)
{
    _ID = ID;
    _filepath = "";
    _type = type;
    _size = size;
}
