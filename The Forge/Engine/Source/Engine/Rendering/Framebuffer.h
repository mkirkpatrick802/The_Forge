#pragma once
#include <memory>

#include "Texture.h"
#include "Engine/Data.h"
#include "glad/glad.h"

namespace Engine
{
    class Texture;

    class Framebuffer
    {
    public:

        Framebuffer(Vector2D size, bool shouldUseRbo);
        ~Framebuffer();

        void Bind() const;
        void Unbind() const;
        
        void Resize(Vector2D size);
        void CheckResize();
        
    private:

        bool Init();
        void CleanUp() const;
        
    public:

        GLuint GetID() const { return _FboID; }
        GLuint GetTextureID() const { return !_texture ? 0 : _texture->GetID(); }
        Vector2D GetSize() const { return _size; }
        
    private:

        GLuint _FboID, _RboID;
        std::shared_ptr<Texture> _texture;
        
        Vector2D _size;
        
        bool _shouldResize, _shouldUseRbo;
    };
}