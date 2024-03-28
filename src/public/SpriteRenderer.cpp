//
// Created by mKirkpatrick on 2/4/2024.
//

#include "SpriteRenderer.h"
#include "ext/matrix_transform.hpp"
#include <sstream>
#include <glm/glm.hpp>
#include "GameObject.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "ext/matrix_clip_space.hpp"

SpriteRenderer::SpriteRenderer()
{

}

void SpriteRenderer::Init()
{
    // configure VAO/VBO
    unsigned int VBO;
    float vertices[] = {
            // pos              // tex
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

void SpriteRenderer::DrawSprite()
{
    float rotation = 0;

    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    _shader.Use();
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(gameObject->_transform.position, 0.0f));

    model = glm::translate(model, glm::vec3(0.5f * _size.x, 0.5f * _size.y, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-0.5f * _size.x, -0.5f * _size.y, 0.0f));

    model = glm::scale(model, glm::vec3(_size, 1.0f));

    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);

    _texture.Bind();

    _shader.SetMatrix4("model", model);
    _shader.SetMatrix4("projection", projection);
    _shader.SetInteger("image", _texture.ID);

    glBindVertexArray(_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //glBindVertexArray(0);
}

void SpriteRenderer::LoadData(const json &data)
{
    std::string texture = data["Texture"];

    int width, height, nrChannels;
    unsigned char* textureData = stbi_load(texture.c_str(), &width, &height, &nrChannels, 0);
    _texture.Generate(width, height, textureData);
    _size = Vector2D(width, height);

    stbi_image_free(textureData);

    _shader.Compile("assets/shaders/Sprite.vert", "assets/shaders/Sprite.frag");
    _sortingLayer = data.value("Sorting Layer", 0);
}
