#pragma once
#include <memory>
#include <string>

#include "UIElement.h"
#include "Engine/Components/ComponentUtils.h"

namespace Engine
{
    // What a UIWidget draws.
    enum class EUIWidgetType : uint8_t { Image = 0, Text, Button };

    const char* UIWidgetTypeName(EUIWidgetType type);

    // A UI element authored on a GameObject, so UI can be laid out in the editor.
    //
    // The bridge between two deliberately separate things. UIElement is not a Component
    // and never will be -- the loading screen has to exist when no level is loaded at
    // all, so the UI layer cannot depend on GameObjects. But a *level's* UI wants
    // everything the editor already provides: the hierarchy, the details panel, prefabs,
    // JSON serialization. This component owns an element and registers it with
    // UIRoot's world canvas, which gets both without either side depending on the other.
    //
    // Note what it deliberately does not do: it is not an IRenderable and never joins the
    // render list. UI is drawn after the scene is composited, by the canvas, in its own
    // projection -- going through the render list would put it back inside the camera's,
    // which is the problem this whole layer exists to solve.
    class UIWidget : public Component
    {
    public:
        UIWidget();
        ~UIWidget() override;

        void OnActivation() override;
        void OnDeactivation() override;

        void Deserialize(const json& data) override;
        nlohmann::json Serialize() override;
        void DrawDetails() override;

        void Write(NetCode::OutputByteStream& stream) const override;
        void Read(NetCode::InputByteStream& stream) override;

        UIElement* GetElement() const { return _element.get(); }

    private:
        // Builds the element for the current type and applies every stored property to
        // it. Called whenever the type changes, because the element's class is what the
        // type selects.
        void RebuildElement();

        void ApplyProperties() const;

        void Register();
        void Unregister();

    private:
        std::unique_ptr<UIElement> _element;
        bool _registered = false;

        // Stored here rather than only on the element, because the element is destroyed
        // and rebuilt whenever the type changes -- and an author who set a colour, then
        // switched from text to button, should not lose it.
        EUIWidgetType _type = EUIWidgetType::Image;

        UIRect _rect;
        glm::vec3 _color = glm::vec3(1.0f);
        float _opacity = 1.0f;

        std::string _spritePath;
        std::string _text;
        unsigned int _fontSize = 12;
        int _align = 0;

        // Looked up in UIActionRegistry when the button is clicked. A name rather than a
        // pointer, because this travels through a JSON file.
        std::string _action;
    };

    REGISTER_COMPONENT(UIWidget)
}
