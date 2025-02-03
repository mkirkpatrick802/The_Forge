#pragma once
#include <memory>

#include "Texture.h"
#include "glad/glad.h"

namespace Engine
{
    class Texture;

    class Framebuffer
    {
    public:

        Framebuffer(glm::vec2 size, bool shouldUseRbo);
        ~Framebuffer();

        void Bind() const;
        void Unbind() const;
        
        void Resize(glm::vec2 size);
        void CheckResize();
        
    private:

        bool Init();
        void CleanUp() const;
        
    public:

        GLuint GetID() const { return _FboID; }
        GLuint GetTextureID() const { return !_texture ? 0 : _texture->GetID(); }
        glm::vec2 GetSize() const { return _size; }
        
    private:

        GLuint _FboID, _RboID;
        std::shared_ptr<Texture> _texture;
        
        glm::vec2 _size;
        
        bool _shouldResize, _shouldUseRbo;
    };
}