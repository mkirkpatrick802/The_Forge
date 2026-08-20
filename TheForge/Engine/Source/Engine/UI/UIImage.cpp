#include "UIImage.h"

#include <glm/ext/matrix_transform.hpp>

#include "UIResources.h"
#include "glad/glad.h"
#include "Engine/AssetMetadata.h"
#include "Engine/System.h"
#include "Engine/Rendering/Texture.h"
#include "Engine/Rendering/TextureLoader.h"

Engine::UIImage::UIImage() = default;

Engine::UIImage::UIImage(const std::string& spritePath)
{
    SetSprite(spritePath);
}

Engine::UIImage::~UIImage() = default;

void Engine::UIImage::SetSprite(const std::string& spritePath)
{
    if (_spritePath == spritePath) return;

    _spritePath = spritePath;

    // Deliberately not loaded here. The texture is built on the next draw, so that
    // setting a sprite is safe from anywhere -- including a level load, which must
    // touch no GL at all.
    InvalidateResources();

    if (_rect.size == glm::vec2(0.0f))
        SizeToSprite();
}

void Engine::UIImage::SizeToSprite()
{
    if (_spritePath.empty()) return;

    // The sidecar, not the image. GetImageSize falls back to reading the file header if
    // an asset predates the import step, and returns (0,0) if it can do neither -- in
    // which case leaving the size alone beats collapsing the element to nothing.
    if (const glm::vec2 size = GetImageSize(_spritePath); size.x > 0.0f && size.y > 0.0f)
        _rect.size = size;
}

const Engine::Texture* Engine::UIImage::ResolveTexture() const
{
    if (_texture) return _texture.get();

    // No sprite means a solid rectangle, not an invisible one.
    return _spritePath.empty() ? UIResources::GetWhiteTexture() : nullptr;
}

void Engine::UIImage::EnsureResourcesResident()
{
    UIElement::EnsureResourcesResident();

    if (_texture || _spritePath.empty()) return;

    _texture = CreateTexture(_spritePath);

    // CreateTexture already logs to the error file, but a UI element that silently
    // fails to load is invisible on screen and looks like a layout bug rather than a
    // missing file. Say which file, once -- _texture stays null, so this would repeat
    // every frame without the guard the caller's early-out provides via _spritePath.
    if (!_texture)
    {
        DEBUG_LOG("UI: could not load image '%s'.", _spritePath.c_str())
        _spritePath.clear();
    }
}

void Engine::UIImage::InvalidateResources()
{
    UIElement::InvalidateResources();
    _texture.reset();
}

void Engine::UIImage::DrawQuad(const glm::mat4& projection, const glm::vec2& topLeft, const glm::vec2& size,
                               const glm::vec3& tint, const float opacity) const
{
    const Texture* texture = ResolveTexture();
    if (texture == nullptr || size.x <= 0.0f || size.y <= 0.0f) return;

    Shader& shader = UIResources::GetSpriteShader();
    if (!shader.IsValid()) return;

    shader.Use();

    auto model = glm::mat4(1.0f);
    model = translate(model, glm::vec3(topLeft, 0.0f));
    model = scale(model, glm::vec3(size, 1.0f));

    // The canvas projection, and an identity view. A UI element is in screen space by
    // definition -- running it through the camera's view matrix is what would make a
    // menu scroll away when the player moved.
    shader.SetMatrix4("projection", projection);
    shader.SetMatrix4("view", glm::mat4(1.0f));
    shader.SetMatrix4("model", model);
    shader.SetVector3f("sprite_color", tint);
    shader.SetFloat("opacity", opacity);
    shader.SetInteger("image", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->GetID());

    glBindVertexArray(UIResources::GetQuadVAO());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Engine::UIImage::Draw(const glm::mat4& projection, const glm::vec2& canvasSize)
{
    if (!IsVisible()) return;

    DrawQuad(projection, _rect.ResolvePosition(canvasSize), _rect.size, _tint, _opacity);
}
