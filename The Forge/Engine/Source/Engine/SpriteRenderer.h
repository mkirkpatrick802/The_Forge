#pragma once
#include "Data.h"
#include "Component.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"

namespace Engine
{
    const int PIXEL_SCALE = 2;

    class SpriteRenderer : public Component
    {
        friend class Renderer;

    public:
        static const uint32 ComponentID = SPRITE_RENDERER;

        SpriteRenderer();
        
        virtual void LoadData(const json& data) override;
        virtual nlohmann::json SaveData() override;

        Vector2D GetSize() const { return _size; }

    private:

        void Init();
        void DrawSprite();

        void CleanUp() override;

    public:
        
        virtual uint32 GetComponentID() const override { return ComponentID; }

    private:
        
        Shader _shader;
        std::shared_ptr<Texture> _texture;

        unsigned int _quadVAO;

        int16        _sortingLayer = 0;
        Vector2D     _size = Vector2D(64);

    };
}