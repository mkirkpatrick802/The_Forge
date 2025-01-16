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

        Framebuffer(Vector2D refRes, bool shouldUseRbo);
        ~Framebuffer();

        void Bind() const;
        void Unbind() const;
        
        void Resize(Vector2D windowSize);
        void CheckResize();
        
    private:

        bool Init();
        void CleanUp() const;
        
    public:

        GLuint GetID() const { return _FboID; }
        GLuint GetTextureID() const { return !_texture ? 0 : _texture->GetID(); }
        Vector2D GetSize() const { return _size; }
        Vector2D GetWindowSize() const { return _windowSize; }
        
    private:

        GLuint _FboID, _RboID;
        std::shared_ptr<Texture> _texture;
        
        Vector2D _windowSize;
        Vector2D _refRes;
        Vector2D _size;
        
        bool _shouldResize, _shouldUseRbo;
    };
}