#include "LineRenderer.h"

#include <glm/ext/matrix_transform.hpp>

#include "imgui.h"
#include "Engine/GameEngine.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Rendering/CameraHelper.h"
#include "Engine/Rendering/CameraManager.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/TextureLoader.h"

Engine::LineRenderer::LineRenderer(): _start(), _end()
{
}

void Engine::LineRenderer::CollectUniforms(ShaderUniformData& data)
{
    
}

void Engine::LineRenderer::Render(const ShaderUniformData& data)
{
    if (!_texture || _hidden) return;
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
    _shader.Use();
    auto model = glm::mat4(1.0f);
    auto screenPos = GetCameraManager().ConvertWorldToScreen(glm::vec2(pos.x - size.x / 2.f, (pos.y * -1) - size.y / 2.f));
    model = translate(model, glm::vec3(screenPos, 0.0f));

    model = translate(model, glm::vec3(.5f * size.x, .5f * size.y, 0.0f));
    model = rotate(model, atan2(dir.x, dir.y), glm::vec3(0.0f, 0.0f, 1.0f));
    model = translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

    model = scale(model, glm::vec3(size, 1.0f));
    
    glBindTextureUnit(0, _texture->GetID());

    _shader.SetMatrix4("model", model);
    
    _shader.SetMatrix4("projection", GetProjectionMatrix());
    _shader.SetMatrix4("view", GetViewMatrix());
    _shader.SetInteger("image", 0);
    
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Engine::LineRenderer::DrawDetails()
{
    SpriteRenderer::DrawDetails();

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
