//
// Created by mKirkpatrick on 2/4/2024.
//

#include "SpriteRenderer.h"
#include <sstream>
#include <GL/glew.h>
#include <glm/glm.hpp>

SpriteRenderer::SpriteRenderer()
{

}

void SpriteRenderer::Init()
{
    // configure VAO/VBO
    unsigned int VBO;
    float vertices[] = {
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &this->_quadVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(this->_quadVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SpriteRenderer::DrawSprite()
{

}

void SpriteRenderer::LoadData(const json &data)
{
    std::string texture = data["Texture"];
    _sortingLayer = data.value("Sorting Layer", 0);

    std::string size = data.value("Size", "16 16");
    std::istringstream iss(size);
    iss >> _size.x >> _size.y;

    _shader.Compile("shaders/Sprite.vert", "shaders/Sprite.frag");

}
