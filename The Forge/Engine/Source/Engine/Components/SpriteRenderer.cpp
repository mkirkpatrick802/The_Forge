#include "SpriteRenderer.h"

#include "Engine/GameObject.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "ComponentRegistry.h"
#include "imgui.h"
#include "Engine/GameEngine.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Rendering/CameraHelper.h"
#include "Engine/Rendering/CameraManager.h"
#include "Engine/Rendering/ImGuiHelper.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/RenderingUtils.h"
#include "Engine/Rendering/TextureLoader.h"

Engine::SpriteRenderer::SpriteRenderer(): _size(), _isScreenSpace(false), _screenSpace()
{
}

void Engine::SpriteRenderer::OnActivation()
{
    InitRenderable(gameObject);
}

void Engine::SpriteRenderer::CollectUniforms(ShaderUniformData& data)
{
    if (!_texture || IsHidden()) return;

    const float rotation = gameObject->transform.rotation;
    glm::vec2 position;

    // Determine position based on space type
    if (_isScreenSpace)
    {
        position = _screenSpace;  // Use screen position directly
    }
    else
    {
        position = glm::vec2(gameObject->transform.position.x - _size.x / 2, 
                             (gameObject->transform.position.y * -1) - _size.y / 2);
        position = GetCameraManager().ConvertWorldToScreen(position);
    }

    auto model = glm::mat4(1.0f);
    model = translate(model, glm::vec3(position, 0.0f));

    // Apply rotation around the center
    model = translate(model, glm::vec3(0.5f * _size.x, 0.5f * _size.y, 0.0f));
    model = rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = translate(model, glm::vec3(-0.5f * _size.x, -0.5f * _size.y, 0.0f));

    // Apply scaling
    model = scale(model, glm::vec3(_size, 1.0f));

    glBindTextureUnit(0, _texture->GetID());

    data.intUniforms["image"] = 0;
    data.mat4Uniforms["model"] = model;
    data.mat4Uniforms["projection"] = GetProjectionMatrix();
    
    // Set view matrix appropriately
    if (_isScreenSpace) {
        data.mat4Uniforms["view"] = glm::mat4(1.0f); // No view matrix in screen space
    } else {
        data.mat4Uniforms["view"] = GetViewMatrix();
    }

}

void Engine::SpriteRenderer::Render(const ShaderUniformData& data)
{
    if (!_texture || IsHidden()) return;
    
    shader.Use();
    
    for (const auto& [name, value] : data.intUniforms)
        shader.SetInteger(name.c_str(), value);
    
    for (const auto& [name, value] : data.floatUniforms)
        shader.SetFloat(name.c_str(), value);
    
    for (const auto& [name, value] : data.vec2Uniforms)
        shader.SetVector2f(name.c_str(), value);
    
    for (const auto& [name, value] : data.vec3Uniforms)
        shader.SetVector3f(name.c_str(), value);
    
    for (const auto& [name, value] : data.vec4Uniforms)
        shader.SetVector4f(name.c_str(), value);

    for (const auto& [name, value] : data.mat4Uniforms)
        shader.SetMatrix4(name.c_str(), value);
    
    glBindVertexArray(GetVAO());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// Serialization 
void Engine::SpriteRenderer::Deserialize(const json& data)
{
    IRenderable::Deserialize(data);
    
    const std::string filepath = data[JsonKeywords::SPRITE_RENDERER_SPRITE];
    _texture = CreateTexture(filepath, Texture::TextureType::PIXEL);
    _size = _texture->GetSize();

    if (data.contains("Is Screen Space"))
        _isScreenSpace = data["Is Screen Space"];

    if (data.contains("Screen Space X") && data.contains("Screen Space Y"))
    {
        _screenSpace.x = data["Screen Space X"];
        _screenSpace.y = data["Screen Space Y"];
    }
    
    GetRenderer().AddComponentToRenderList(this);
}

nlohmann::json Engine::SpriteRenderer::Serialize()
{
    nlohmann::json data = IRenderable::Serialize();

    if (_texture)
    {
        std::string filepath = _texture->GetFilePath();
        data[JsonKeywords::SPRITE_RENDERER_SPRITE] = filepath;
    }

    data["Is Screen Space"] = _isScreenSpace;
    data["Screen Space X"] = _screenSpace.x;
    data["Screen Space Y"] = _screenSpace.y;
    
    return data;
}

void Engine::SpriteRenderer::Write(NetCode::OutputByteStream& stream) const
{
    IRenderable::Write(stream);
    stream.Write(_texture->GetFilePath());
}

void Engine::SpriteRenderer::Read(NetCode::InputByteStream& stream)
{
    IRenderable::Read(stream);
    
    std::string filepath;
    stream.Read(filepath);

    if (!_texture)
    {
        _texture = CreateTexture(filepath, Texture::TextureType::PIXEL);
        _size = _texture->GetSize();
    }
}

void Engine::SpriteRenderer::DrawDetails()
{
   DrawRenderableSettings();

    if (std::string path; ImGuiHelper::DragDropFileButton("Sprite", path, "FILE_PATH"))
    {
        _texture.reset();
        _texture = CreateTexture(path, Texture::TextureType::PIXEL);
        _size = _texture->GetSize();
    }

    ImGuiHelper::DisplayFilePath("Sprite Saved Path", _texture ? _texture->GetFilePath() : "");
    ImGui::Spacing();

    ImGui::Checkbox("Is Screen Space", &_isScreenSpace);
    ImGui::Spacing();
    if (_isScreenSpace)
    {
        ImGuiHelper::InputVector2("Screen Location", _screenSpace);
        ImGui::Spacing();
    }
}
