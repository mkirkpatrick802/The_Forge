//
// Created by mKirkpatrick on 2/4/2024.
//

#include "SpriteRenderer.h"
#include "GameObject.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Renderer.h"

Engine::SpriteRenderer::SpriteRenderer(): _quadVAO(0)
{
    _texture = Texture();
    
    // TODO: Get this from renderer
    _projection = glm::ortho(0.0f, 1280.f, 720.f, 0.0f, -1.0f, 1.0f);
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
}

void Engine::SpriteRenderer::LoadData(const json& data)
{
    const String filepath = data["Texture"];
    
    _texture = Texture(filepath.c_str());
    
    String vertex = data["Vertex Shader"];
    String fragment = data["Fragment Shader"];

    _shader.Compile(vertex.c_str(), fragment.c_str());
    _sortingLayer = (int16)data.value("Sorting Layer", 0);
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

    glBindTextureUnit(0, _texture.ID);

    _shader.SetMatrix4("model", model);
    _shader.SetMatrix4("projection", _projection);
    _shader.SetInteger("image", 0);

    TriggerCallback(_shader);

    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void Engine::SpriteRenderer::OnDestroyed()
{
	Component::OnDestroyed();

    callbacks.clear();
}

void Engine::SpriteRenderer::RegisterCallback(const ShaderCallbackFunction& function)
{
    callbacks.push_back(function);
}

void Engine::SpriteRenderer::TriggerCallback(Shader& shader) const
{
    for (auto& callback : callbacks)
        callback(shader);
}
