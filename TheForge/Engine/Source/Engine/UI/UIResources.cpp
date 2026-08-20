#include "UIResources.h"

#include "glad/glad.h"
#include "Engine/System.h"
#include "Engine/Rendering/Font.h"
#include "Engine/Rendering/Texture.h"
#include "Engine/Rendering/RenderingUtils.h"

unsigned int Engine::UIResources::_quadVAO = 0;
std::unique_ptr<Shader> Engine::UIResources::_spriteShader;
std::unique_ptr<Engine::Texture> Engine::UIResources::_whiteTexture;
std::map<std::pair<std::string, unsigned int>, std::unique_ptr<Engine::Font>> Engine::UIResources::_fonts;

unsigned int Engine::UIResources::GetQuadVAO()
{
    if (_quadVAO == 0)
        _quadVAO = RenderingUtils::GenerateVAO();

    return _quadVAO;
}

Shader& Engine::UIResources::GetSpriteShader()
{
    if (!_spriteShader)
    {
        _spriteShader = std::make_unique<Shader>();
        _spriteShader->Compile(UI_SPRITE_VERTEX_SHADER.c_str(), UI_SPRITE_FRAGMENT_SHADER.c_str());

        // Worth saying out loud. A UI element whose shader failed to compile draws
        // nothing at all, which on screen is indistinguishable from the element never
        // having been added -- and the usual cause is the working directory, not the
        // shader.
        if (!_spriteShader->IsValid())
            DEBUG_LOG("UI: the sprite shader failed to compile. Nothing on the UI layer will draw.")
    }

    return *_spriteShader;
}

Engine::Texture* Engine::UIResources::GetWhiteTexture()
{
    if (!_whiteTexture)
    {
        constexpr unsigned char pixel[4] = {255, 255, 255, 255};

        GLuint id;
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

        // Clamped, so stretching one pixel across a panel cannot sample past its own
        // edge and pick up the wrap. Nearest, because there is nothing to interpolate.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        _whiteTexture = std::make_unique<Texture>(id, Texture::TextureType::PIXEL, glm::vec2(1.0f));
    }

    return _whiteTexture.get();
}

Engine::Font* Engine::UIResources::GetFont(const std::string& fontPath, const unsigned int fontSize)
{
    // A zero-sized font makes FreeType produce empty glyphs, so every label built before
    // its size was set would silently draw nothing.
    const unsigned int size = fontSize == 0 ? 1 : fontSize;

    const auto key = std::make_pair(fontPath, size);
    if (const auto existing = _fonts.find(key); existing != _fonts.end())
        return existing->second.get();

    auto font = std::make_unique<Font>(fontPath, size);
    Font* raw = font.get();
    _fonts.emplace(key, std::move(font));

    return raw;
}

void Engine::UIResources::CleanUp()
{
    _fonts.clear();

    if (_whiteTexture)
    {
        const GLuint id = _whiteTexture->GetID();
        glDeleteTextures(1, &id);
        _whiteTexture.reset();
    }

    _spriteShader.reset();

    if (_quadVAO != 0)
    {
        glDeleteVertexArrays(1, &_quadVAO);
        _quadVAO = 0;
    }
}
