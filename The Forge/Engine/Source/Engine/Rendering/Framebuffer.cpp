#include "Framebuffer.h"

#include "TextureLoader.h"
#include "Engine/System.h"

Engine::Framebuffer::Framebuffer(const Vector2D size, const bool shouldUseRbo)
{
    _size = size;
    _shouldUseRbo = shouldUseRbo;

    _texture = CreateTexture(size, Texture::TextureType::FRAMEBUFFER);
    if (!_texture || !Init()) System::DisplayMessageBox("Error", "Failed to create or init framebuffer texture!!!");
}

bool Engine::Framebuffer::Init()
{
    glCreateFramebuffers(1, &_FboID);
    glBindFramebuffer(GL_FRAMEBUFFER, _FboID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture->GetID(), 0);

    if (_shouldUseRbo)
    {
        glCreateRenderbuffers(1, &_RboID);
        glBindRenderbuffer(GL_RENDERBUFFER, _RboID);

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, (int)_size.x, (int)_size.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _RboID);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer error: " << std::hex << status << '\n';
        }
        
        assert(false && "Failed to create framebuffer!!");
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void Engine::Framebuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, _FboID);
}

void Engine::Framebuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::Framebuffer::Resize(const Vector2D size)
{
    _size = size;
    _shouldResize = true;
}

void Engine::Framebuffer::CheckResize()
{
    if (!_shouldResize) return;

    CleanUp();
    _texture.reset();
    
    _texture = CreateTexture(_size, Texture::TextureType::FRAMEBUFFER);
    Init();
    
    _shouldResize = false;
}

Engine::Framebuffer::~Framebuffer()
{
    CleanUp();
}

void Engine::Framebuffer::CleanUp() const
{
    glDeleteFramebuffers(1, &_FboID);
    if (_shouldUseRbo)
        glDeleteRenderbuffers(1, &_RboID);

    if (_texture)
    {
        const auto texture = _texture->GetID();
        glDeleteTextures(1, &texture);
    }
}
