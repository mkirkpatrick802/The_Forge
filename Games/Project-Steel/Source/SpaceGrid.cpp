#include "SpaceGrid.h"

#include <glm/ext/matrix_transform.hpp>

#include "Engine/Rendering/CameraHelper.h"
#include "Engine/Rendering/CameraManager.h"

SpaceGrid::SpaceGrid(): _gridSize(2,2)
{
    
}

void SpaceGrid::OnActivation()
{
    SpriteRenderer::OnActivation();
}

void SpaceGrid::Render(const Engine::ShaderUniformData& data)
{
    if (!_texture || _hidden) return;

    for (int x = 0; x < _gridSize.x; x++)
    {
        for (int y = 0; y < _gridSize.y; y++)
        {
            glm::vec2 pos(x * _size.x, y * _size.y);
            RenderTile(pos + gameObject->transform.position);
        }
    }
}

void SpaceGrid::RenderTile(glm::vec2 pos)
{
    _shader.Use();
    auto model = glm::mat4(1.0f);
    auto screenPos = Engine::GetCameraManager().ConvertWorldToScreen(glm::vec2(pos.x - _size.x / 2.f, (pos.y * -1) - _size.y / 2.f));
    model = translate(model, glm::vec3(screenPos, 0.0f));

    model = translate(model, glm::vec3(.5f * _size.x, .5f * _size.y, 0.0f));
    model = rotate(model, 0.f, glm::vec3(0.0f, 0.0f, 1.0f));
    model = translate(model, glm::vec3(-0.5f * _size.x, -0.5f * _size.y, 0.0f));

    model = scale(model, glm::vec3(_size, 1.0f));
    
    glBindTextureUnit(0, _texture->GetID());

    _shader.SetMatrix4("model", model);
    
    _shader.SetMatrix4("projection", Engine::GetProjectionMatrix());
    _shader.SetMatrix4("view", Engine::GetViewMatrix());
    _shader.SetInteger("image", 0);
    
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SpaceGrid::DrawDetails()
{
    SpriteRenderer::DrawDetails();
}