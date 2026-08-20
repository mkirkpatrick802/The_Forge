#include "UIButton.h"

#include "Engine/Rendering/Texture.h"

Engine::UIButton::UIButton() = default;
Engine::UIButton::~UIButton() = default;

Engine::UIButton::UIButton(const std::string& label, std::function<void()> onClick)
    : _onClick(std::move(onClick))
{
    _label.SetText(label);
    _label.SetAlign(EUITextAlign::Center);
}

void Engine::UIButton::SetStateTints(const glm::vec3& normal, const glm::vec3& hover, const glm::vec3& pressed)
{
    _normalTint = normal;
    _hoverTint = hover;
    _pressedTint = pressed;
}

void Engine::UIButton::SetBackground(const glm::vec3& color, const float opacity)
{
    _backgroundColor = color;
    _backgroundOpacity = opacity;
    _drawBackground = true;
}

void Engine::UIButton::InvalidateResources()
{
    UIImage::InvalidateResources();
    _label.InvalidateResources();
}

void Engine::UIButton::OnPointerMoved(const glm::vec2& canvasPoint, const glm::vec2& canvasSize)
{
    // Told to every element every frame, not only the one under the cursor, so a button
    // the pointer has left hears about it and stops drawing lit.
    _hovered = _enabled && HitTest(canvasPoint, canvasSize);

    if (!_hovered && !_enabled)
        _pressed = false;
}

bool Engine::UIButton::OnPointerPressed(const glm::vec2& canvasPoint, const glm::vec2& canvasSize)
{
    if (!_enabled || !IsVisible()) return false;

    _pressed = true;

    // Claims the press, which stops the canvas offering it to anything behind and is
    // what makes UIRoot consume the click for the frame.
    return true;
}

void Engine::UIButton::OnPointerReleased(const glm::vec2& canvasPoint, const glm::vec2& canvasSize)
{
    const bool wasPressed = _pressed;
    _pressed = false;

    if (!wasPressed || !_enabled) return;

    // Released off the button is a cancel, not a click. Pressing a button and dragging
    // away to think better of it is a convention old enough that violating it reads as
    // a bug.
    if (!HitTest(canvasPoint, canvasSize)) return;

    if (_onClick)
        _onClick();
}

void Engine::UIButton::Draw(const glm::mat4& projection, const glm::vec2& canvasSize)
{
    if (!IsVisible()) return;

    const glm::vec3 stateTint =
        !_enabled ? _disabledTint :
        _pressed  ? _pressedTint  :
        _hovered  ? _hoverTint    : _normalTint;

    // A button with no sprite *and* no explicit background is a text-only menu entry:
    // the state then has to show on the label, or hovering it does nothing visible at
    // all. A sprite or a background colour opts into a drawn panel, which takes the
    // state tint instead.
    const bool hasPanel = _drawBackground || HasSprite();

    if (hasPanel)
    {
        const glm::vec3 panelTint = HasSprite() ? _tint : _tint * _backgroundColor;
        const float panelOpacity = _opacity * (HasSprite() ? 1.0f : _backgroundOpacity);

        DrawQuad(projection, _rect.ResolvePosition(canvasSize), _rect.size, panelTint * stateTint, panelOpacity);
    }

    // The label is laid out against the button's rect every draw rather than being
    // positioned once, so moving or resizing the button carries its text with it.
    _label.Rect().anchor = _rect.anchor;
    _label.Rect().offset = _rect.offset;
    _label.Rect().pivot = _rect.pivot;
    _label.Rect().size = _rect.size;
    _label.SetOpacity(_opacity);

    _label.SetTint(hasPanel ? _tint : _tint * stateTint);

    // Vertically centred within the button. UIText draws from the top of its rect, so
    // without this the label sits on the button's top edge.
    //
    // The font is resolved first: measuring before it exists returns zero, which would
    // leave every label misplaced on the frame a menu opened and correct on the next.
    _label.PrepareForCanvas(canvasSize);
    const glm::vec2 measured = _label.Measure();
    if (measured.y > 0.0f && _rect.size.y > measured.y)
        _label.Rect().offset.y += (_rect.size.y - measured.y) * 0.5f;

    _label.EnsureResourcesResident();
    _label.Draw(projection, canvasSize);
}
