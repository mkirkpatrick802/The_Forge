#pragma once
#include "Component.h"
#include "ComponentUtils.h"
#include "Engine/Rendering/Shader.h"
#include "Engine/Rendering/Texture.h"

namespace Engine
{
    class SpriteRenderer : public Component
    {
        friend class Renderer;

    public:

        SpriteRenderer();
        ~SpriteRenderer() override;

        void OnActivation() override;
        
        virtual void Deserialize(const json& data) override;
        virtual nlohmann::json Serialize() override;

        void Write(NetCode::OutputByteStream& stream) const override;
        void Read(NetCode::InputByteStream& stream) override;
        
        virtual void DrawDetails() override;
        
        glm::vec2 GetSize() const { return _size; }

    private:
        
        void DrawSprite();

    private:
        
        Shader _shader;
        std::unique_ptr<Texture> _texture;

        unsigned int _quadVAO;

        int16_t _sortingLayer = 0;
        glm::vec2 _size = glm::vec2(64);

    };

    REGISTER_COMPONENT(SpriteRenderer)
}