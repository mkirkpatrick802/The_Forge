#include "SpriteRenderer.h"

#include <iostream>

#include "Engine/GameObject.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "ComponentRegistry.h"
#include "imgui.h"
#include "Engine/GameEngine.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Rendering/CameraHelper.h"
#include "Engine/Rendering/CameraManager.h"
#include "Engine/Rendering/Renderer.h"
#include "Engine/Rendering/TextureLoader.h"

Engine::SpriteRenderer::SpriteRenderer(): _quadVAO(0)
{
    
}

void Engine::SpriteRenderer::OnActivation()
{
    // configure VAO/VBO
    unsigned int VBO;
    const float vertices[] = {
        // pos         // tex
        0.0f, 1.0f, 0.0f, 0.0f,  // flipped tex y: 1.0 -> 0.0
        1.0f, 0.0f, 1.0f, 1.0f,  // flipped tex y: 0.0 -> 1.0
        0.0f, 0.0f, 0.0f, 1.0f,  // flipped tex y: 0.0 -> 1.0

        0.0f, 1.0f, 0.0f, 0.0f,  // flipped tex y: 1.0 -> 0.0
        1.0f, 1.0f, 1.0f, 0.0f,  // flipped tex y: 1.0 -> 0.0
        1.0f, 0.0f, 1.0f, 1.0f   // flipped tex y: 0.0 -> 1.0
    };


    glGenVertexArrays(1, &_quadVAO);
    glBindVertexArray(_quadVAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GetRenderer().AddSpriteRendererToRenderList(this);
    
    std::string vertex = "Assets/Shaders/Sprite.vert";
    std::string fragment = "Assets/Shaders/Sprite.frag";

    _shader.Compile(vertex.c_str(), fragment.c_str());
    _sortingLayer = 0;
}

Engine::SpriteRenderer::~SpriteRenderer()
{
    _texture.reset();
    GetRenderer().RemoveSpriteRendererFromRenderList(this);
}

void Engine::SpriteRenderer::Deserialize(const json& data)
{
    const std::string filepath = data[JsonKeywords::SPRITE_RENDERER_SPRITE];
    _texture = CreateTexture(filepath, Texture::TextureType::PIXEL);

    std::string vertex = "Assets/Shaders/Sprite.vert";
    std::string fragment = "Assets/Shaders/Sprite.frag";

    _shader.Compile(vertex.c_str(), fragment.c_str());
    _sortingLayer = (int16_t)data.value(JsonKeywords::SPRITE_RENDERER_SORTING_LAYER, 0);
}

nlohmann::json Engine::SpriteRenderer::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::COMPONENT_ID] = GetComponentRegistry().GetComponentID<SpriteRenderer>();

    if (_texture)
    {
        std::string filepath = _texture->GetFilePath();
        data[JsonKeywords::SPRITE_RENDERER_SPRITE] = filepath;
    }
    
    data[JsonKeywords::SPRITE_RENDERER_SORTING_LAYER] = _sortingLayer;

    // TODO: Save Shader Information
    
    return data;
}

void Engine::SpriteRenderer::DrawDetails()
{
    const char* filePath = nullptr; // Store the dropped file path
    if (ImGui::Button("Drop File Here", ImVec2(200, 50))) {}

    // Drag & Drop Target
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
        {
            filePath = static_cast<const char*>(payload->Data); // Retrieve the file path
            
            _texture.reset();
            _texture = CreateTexture(filePath, Texture::TextureType::PIXEL);
            
            std::cout << "Dropped new file: " << filePath << '\n';
        }
        ImGui::EndDragDropTarget();
    }

    // Display saved file path
    if (!_texture) return;
    if (!_texture->GetFilePath().empty())
    {
        ImGui::Text("Saved Path: %s", _texture->GetFilePath().c_str());
    }
}

void Engine::SpriteRenderer::DrawSprite()
{
	const float rotation = gameObject->transform.rotation;
	const auto position = glm::vec2(gameObject->transform.position.x - _size.x / 2, (gameObject->transform.position.y * -1) - _size.y / 2);
    
    _shader.Use();
	auto model = glm::mat4(1.0f);
    auto screenPos = GetCameraManager().ConvertWorldToScreen(position);
    model = translate(model, glm::vec3(screenPos, 0.0f));

    model = translate(model, glm::vec3(0.5f * _size.x, 0.5f * _size.y, 0.0f));
    model = rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = translate(model, glm::vec3(-0.5f * _size.x, -0.5f * _size.y, 0.0f));

    model = scale(model, glm::vec3(_size, 1.0f));

    if (!_texture) return;
    glBindTextureUnit(0, _texture->GetID());

    _shader.SetMatrix4("model", model);
    
    _shader.SetMatrix4("projection", GetProjectionMatrix());
    _shader.SetMatrix4("view", GetViewMatrix());
    _shader.SetInteger("image", 0);
    
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
