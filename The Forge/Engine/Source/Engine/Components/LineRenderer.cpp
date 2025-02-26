#include "LineRenderer.h"

#include <glm/ext/matrix_transform.hpp>

#include "imgui.h"
#include "Engine/GameEngine.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Rendering/CameraHelper.h"
#include "Engine/Rendering/CameraManager.h"
#include "Engine/Rendering/ImGuiHelper.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/TextureLoader.h"

Engine::LineRenderer::LineRenderer(): _size(), _start(), _end() { }

Engine::LineRenderer::~LineRenderer()
{
    IRenderable::~IRenderable();

    _texture.reset();
    GetRenderer().RemoveComponentFromRenderList(this);
}

void Engine::LineRenderer::OnActivation()
{
    InitRenderable(gameObject);
    GetRenderer().AddComponentToRenderList(this);
}

void Engine::LineRenderer::Render(const ShaderUniformData& data)
{
    if (!_texture || IsHidden()) return;
    glm::vec2 direction = _end - _start;
    float length = glm::length(direction);
    glm::vec2 step = glm::normalize(direction) * _size.x;

    int fullSprites = static_cast<int>(length / _size.x);
    float remainder = fmod(length, _size.x); // Remaining space

    for (int i = 0; i < fullSprites; ++i)
    {
        glm::vec2 position = _start + step * (float)i;
        RenderLine(position, direction, _size);
    }

    // Render last partial sprite
    if (remainder > 0.0f)
    {
        glm::vec2 lastPosition = _start + step * (float)fullSprites;
        float scale = remainder / _size.x;
        RenderLine(lastPosition, direction, _size * scale);
    }
}

void Engine::LineRenderer::RenderLine(glm::vec2 pos, glm::vec2 dir, glm::vec2 size)
{
    shader.Use();
    auto model = glm::mat4(1.0f);
    auto screenPos = GetCameraManager().ConvertWorldToScreen(glm::vec2(pos.x - size.x / 2.f, (pos.y * -1) - size.y / 2.f));
    model = translate(model, glm::vec3(screenPos, 0.0f));

    model = translate(model, glm::vec3(.5f * size.x, .5f * size.y, 0.0f));
    model = rotate(model, atan2(dir.x, dir.y), glm::vec3(0.0f, 0.0f, 1.0f));
    model = translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

    model = scale(model, glm::vec3(size, 1.0f));
    
    glBindTextureUnit(0, _texture->GetID());

    shader.SetMatrix4("model", model);
    
    shader.SetMatrix4("projection", GetProjectionMatrix());
    shader.SetMatrix4("view", GetViewMatrix());
    shader.SetInteger("image", 0);
    
    glBindVertexArray(GetVAO());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Engine::LineRenderer::Deserialize(const json& data)
{
    IRenderable::Deserialize(data);
    const std::string filepath = data[JsonKeywords::SPRITE_RENDERER_SPRITE];
    _texture = CreateTexture(filepath, Texture::TextureType::PIXEL);
    _size = _texture->GetSize();

    GetRenderer().AddComponentToRenderList(this);
}

nlohmann::json Engine::LineRenderer::Serialize()
{
    nlohmann::json data = IRenderable::Serialize();

    if (_texture)
    {
        std::string filepath = _texture->GetFilePath();
        data[JsonKeywords::SPRITE_RENDERER_SPRITE] = filepath;
    }
    
    return data;
}

void Engine::LineRenderer::Write(NetCode::OutputByteStream& stream) const
{
    IRenderable::Write(stream);
    stream.Write(_texture->GetFilePath());
    stream.Write(_start);
    stream.Write(_end);
}

void Engine::LineRenderer::Read(NetCode::InputByteStream& stream)
{
    IRenderable::Read(stream);
    
    std::string filepath;
    stream.Read(filepath);

    if (!_texture)
    {
        _texture = CreateTexture(filepath, Texture::TextureType::PIXEL);
        _size = _texture->GetSize();
    }

    stream.Read(_start);
    stream.Read(_end);

    GetRenderer().AddComponentToRenderList(this);
}

void Engine::LineRenderer::DrawDetails()
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

    ImGui::Spacing();
    ImGui::Text("Start Point:");
    ImGui::PushItemWidth(80);
    ImGui::DragFloat("##Start X", &_start.x, 1.0f);
    ImGui::SameLine();
    ImGui::DragFloat("##Start Y", &_start.y, 1.0f);
    ImGui::PopItemWidth();

    ImGui::Spacing();
    ImGui::Text("End Point:");
    ImGui::PushItemWidth(80);
    ImGui::DragFloat("##End X", &_end.x, 1.0f);
    ImGui::SameLine();
    ImGui::DragFloat("##End Y", &_end.y, 1.0f);
    ImGui::PopItemWidth();
}
