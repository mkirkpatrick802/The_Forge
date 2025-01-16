#pragma once
#include "Engine/Data.h"
#include "glad/glad.h"

namespace Engine
{
    // TODO: Rework how textures are made.
    // Call a static function from the texture loader that returns a smart pointer to a texture object.
    class Texture
    {
    public:
        
        enum class TextureType { PIXEL = 0, FRAMEBUFFER };

        Texture();
        Texture(TextureType type, const String& filepath);
        Texture(TextureType type, Vector2D size);
        ~Texture() = default;

    public:

        GLuint GetID() const { return _ID; }
        Vector2D GetSize() const { return _size; }
        String GetFilePath() const { return _filepath; }
        
    private:

        GLuint _ID;
        String _filepath;
        Vector2D _size;
        TextureType _type;
    };
}
