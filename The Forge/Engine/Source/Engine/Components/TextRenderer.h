#pragma once
#include "ComponentUtils.h"

namespace Engine
{
    class Font;
    class TextRenderer final : public Component
    {
    public:
        TextRenderer();
        void Deserialize(const json& data) override;
        nlohmann::json Serialize() override;

    private:
        std::unique_ptr<Font> _font;
        std::string _text = "Hello";
        int _fontSize = 40;
        
    };

    REGISTER_COMPONENT(TextRenderer)
}
