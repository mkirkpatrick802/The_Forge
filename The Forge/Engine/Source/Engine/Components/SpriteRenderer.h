#pragma once
#include "Engine/Data.h"
#include "Component.h"
#include "ComponentRegistry.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/Texture.h"

namespace Engine
{
    const int PIXEL_SCALE = 2;

    class SpriteRenderer : public Component
    {
        friend class Renderer;

    public:
        
        SpriteRenderer();
        
        virtual void LoadData(const json& data) override;
        virtual nlohmann::json SaveData() override;

        Vector2D GetSize() const { return _size; }

    private:

        void Init();
        void DrawSprite();

        void CleanUp() override;

    private:
        
        Shader _shader;
        std::shared_ptr<Texture> _texture;

        unsigned int _quadVAO;

        int16        _sortingLayer = 0;
        Vector2D     _size = Vector2D(64);

    };

    REGISTER_COMPONENT(SpriteRenderer, SPRITE_RENDERER)
}