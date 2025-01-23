#include "SpriteRenderer.h"
#include "GameObject.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "EventData.h"
#include "EventSystem.h"
#include "JsonKeywords.h"
#include "Rendering/CameraHelper.h"
#include "Rendering/CameraManager.h"
#include "Rendering/Renderer.h"
#include "Rendering/TextureLoader.h"

Engine::SpriteRenderer::SpriteRenderer(): _quadVAO(0)
{
    RegisterComponent(ComponentID, "Sprite Renderer");
}

void Engine::SpriteRenderer::Init()
{
    // configure VAO/VBO
    unsigned int VBO;
    const float vertices[] = {

            // pos      // tex
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,

            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f
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

    const String filepath = "Assets/Sprites/Astronaut.png";
    
    _texture = CreateTexture(filepath, Texture::TextureType::PIXEL);
    
    String vertex = "Assets/Shaders/Sprite.vert";
    String fragment = "Assets/Shaders/Sprite.frag";

    _shader.Compile(vertex.c_str(), fragment.c_str());
    _sortingLayer = 0;
}

void Engine::SpriteRenderer::LoadData(const json& data)
{
    const String filepath = data[JsonKeywords::SPRITE_RENDERER_SPRITE];
    
    _texture = CreateTexture(filepath, Texture::TextureType::PIXEL);

    String vertex = "Assets/Shaders/Sprite.vert";
    String fragment = "Assets/Shaders/Sprite.frag";
    //String vertex = data[JsonKeywords::SPRITE_RENDERER_VERTEX_SHADER];
    //String fragment = data[JsonKeywords::SPRITE_RENDERER_FRAGMENT_SHADER];

    _shader.Compile(vertex.c_str(), fragment.c_str());
    _sortingLayer = (int16)data.value(JsonKeywords::SPRITE_RENDERER_SORTING_LAYER, 0);
}

nlohmann::json Engine::SpriteRenderer::SaveData()
{
    nlohmann::json data;
    data[JsonKeywords::COMPONENT_ID] = ComponentID;
    String filepath = _texture->GetFilePath();
    data[JsonKeywords::SPRITE_RENDERER_SPRITE] = filepath;
    data[JsonKeywords::SPRITE_RENDERER_SORTING_LAYER] = _sortingLayer;

    // Save Shader Information
    //data[JsonKeywords::SPRITE_RENDERER_VERTEX_SHADER] = 
    return data;
}

void Engine::SpriteRenderer::DrawSprite()
{
	const float rotation = gameObject->transform.rotation;
	const auto position = Vector2D(gameObject->transform.position.x - _size.x / PIXEL_SCALE, (gameObject->transform.position.y * -1) - _size.y / PIXEL_SCALE);
    
    _shader.Use();
	auto model = glm::mat4(1.0f);
    model = translate(model, glm::vec3(Renderer::ConvertWorldToScreen(position), 0.0f));

    model = translate(model, glm::vec3(0.5f * _size.x, 0.5f * _size.y, 0.0f));
    model = rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = translate(model, glm::vec3(-0.5f * _size.x, -0.5f * _size.y, 0.0f));

    model = scale(model, glm::vec3(_size, 1.0f));

    glBindTextureUnit(0, _texture->GetID());

    _shader.SetMatrix4("model", model);
    
    _shader.SetMatrix4("projection", GetProjectionMatrix());
    _shader.SetMatrix4("view", GetViewMatrix());
    _shader.SetInteger("image", 0);
    
    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Engine::SpriteRenderer::CleanUp()
{
    ED_DestroyComponent eventData;
    eventData.component = this;
    
    EventSystem::GetInstance()->TriggerEvent(ED_DestroyComponent::GetEventName(), &eventData);
}
