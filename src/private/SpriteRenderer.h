//
// Created by mKirkpatrick on 1/30/2024.
//
#pragma once

#include "GameData.h"
#include "Component.h"
#include <SDL_render.h>
#include "Shader.h"
#include "Texture.h"

class Renderer;

const int PIXEL_SCALE = 2;

class SpriteRenderer : public Component {

    friend class Renderer;

public:

    const static uint32 ComponentID = SPRITE_RENDERER;

    SpriteRenderer();
    virtual void LoadData(const json& data) override;

    Vector2D GetSize() const { return _size; }

private:

    void Init();
    void DrawSprite();

private:

    Shader       _shader;
    Texture      _texture;

    unsigned int _quadVAO;

    glm::mat4 _projection;

    //Texture2D     _texture;
    int16        _sortingLayer = 0;
    Vector2D     _size = Vector2D(16);

};