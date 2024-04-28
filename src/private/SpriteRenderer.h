#pragma once

#include "GameData.h"
#include "Component.h"
#include "Shader.h"
#include "Texture.h"

class Renderer;

const int PIXEL_SCALE = 2;

using ShaderCallbackFunction = std::function<void(Shader&)>;

class SpriteRenderer : public Component
{
    friend class Renderer;

public:

    const static uint32 ComponentID = SPRITE_RENDERER;

    SpriteRenderer();
    virtual void LoadData(const json& data) override;

    Vector2D GetSize() const { return _size; }

    void RegisterCallback(const ShaderCallbackFunction& function);
    void TriggerCallback(Shader& shader) const;

private:

    void Init();
    void DrawSprite();

public:

    virtual void OnDestroyed() override;

private:

    std::vector<ShaderCallbackFunction> callbacks;

    Shader _shader;
    Texture _texture;

    unsigned int _quadVAO;

    glm::mat4 _projection;

    int16        _sortingLayer = 0;
    Vector2D     _size = Vector2D(16);

};