#pragma once
#include "Component.h"
#include "ComponentUtils.h"
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
        ~SpriteRenderer();
        
        virtual void Deserialize(const json& data) override;
        virtual nlohmann::json Serialize() override;

        glm::vec2 GetSize() const { return _size; }

    private:

        void Init();
        void DrawSprite();

    private:

        bool _isInitialized = false;
        Shader _shader;
        std::unique_ptr<Texture> _texture;

        unsigned int _quadVAO;

        int16_t _sortingLayer = 0;
        glm::vec2 _size = glm::vec2(64);

    };

    REGISTER_COMPONENT(SpriteRenderer)
}