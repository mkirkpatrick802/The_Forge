#pragma once
#include <functional>
#include <string>

#include "UIImage.h"
#include "UIText.h"

namespace Engine
{
    // A clickable button: a background image, a label, and a callback.
    //
    // Composed rather than inherited from both -- a UIImage that also happens to be a
    // UIText would have two rects and two ideas of where it is. The label is owned and
    // positioned against the button's own rect on every draw, so moving the button moves
    // its text.
    class UIButton final : public UIImage
    {
    public:
        // Declared here, defined in the .cpp -- never defaulted inline. See the note on
        // UIImage's constructors: the inherited unique_ptr<Texture> is against a
        // forward declaration, so anything the compiler generates at a call site would
        // need a complete Texture there.
        UIButton();
        explicit UIButton(const std::string& label, std::function<void()> onClick = nullptr);
        ~UIButton() override;

        void Draw(const glm::mat4& projection, const glm::vec2& canvasSize) override;
        void InvalidateResources() override;

        bool OnPointerPressed(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) override;
        void OnPointerReleased(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) override;
        void OnPointerMoved(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) override;

        void SetLabel(const std::string& label) { _label.SetText(label); }
        const std::string& GetLabel() const { return _label.GetText(); }

        UIText& Label() { return _label; }

        void SetOnClick(std::function<void()> onClick) { _onClick = std::move(onClick); }

        // A disabled button is drawn dimmed and takes no input, but still occupies its
        // space -- which is what keeps a menu from reflowing as options come and go.
        void SetEnabled(const bool enabled) { _enabled = enabled; }
        bool IsEnabled() const { return _enabled; }

        // Draws a solid panel behind the label in the given colour. Without this a
        // button is text only, which is what a plain menu wants; with it, it is a
        // conventional filled button. A button with a sprite ignores the colour and
        // tints the sprite instead.
        void SetBackground(const glm::vec3& color, float opacity = 1.0f);
        void ClearBackground() { _drawBackground = false; }

        // Tints applied to the background in each state. Multiplied with the element's
        // own tint, so a red button still reads as red when hovered.
        void SetStateTints(const glm::vec3& normal, const glm::vec3& hover, const glm::vec3& pressed);

    private:
        UIText _label;
        std::function<void()> _onClick;

        // Normal is deliberately *below* full brightness so that hover has somewhere to
        // go. Brightening past 1.0 clamps, which on a white label means hover looks
        // identical to idle -- the button gives no feedback at all, and the menu feels
        // broken rather than merely plain. Resting dim and lighting to full works for a
        // text-only button and a sprite-backed one alike.
        glm::vec3 _normalTint = glm::vec3(0.72f);
        glm::vec3 _hoverTint = glm::vec3(1.0f);
        glm::vec3 _pressedTint = glm::vec3(0.5f);
        glm::vec3 _disabledTint = glm::vec3(0.35f);

        glm::vec3 _backgroundColor = glm::vec3(1.0f);
        float _backgroundOpacity = 1.0f;
        bool _drawBackground = false;

        bool _enabled = true;
        bool _hovered = false;
        bool _pressed = false;
    };
}
