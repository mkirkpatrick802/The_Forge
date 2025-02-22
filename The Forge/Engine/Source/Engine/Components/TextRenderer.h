﻿#pragma once
#include "ComponentUtils.h"
#include "Engine/Rendering/Font.h"

namespace Engine
{
    class TextRenderer : public Component
    {
    public:
        TextRenderer();
        ~TextRenderer() override;
        void OnActivation() override;
        void Render(const ShaderUniformData& data) override;
        void Deserialize(const json& data) override;
        nlohmann::json Serialize() override;
        void DrawDetails() override;

    protected:
        std::unique_ptr<Font> _font;
        glm::vec2 _screenPos = glm::vec2(0.0f); 
        std::string _text = "Default Text";
        int _fontSize = 50;

    public:
        inline void SetText(const std::string& text) { _text = text; }
        
    };

    REGISTER_COMPONENT(TextRenderer)
}
