#include "UISlider.h"

#include <algorithm>

#include "Engine/Rendering/Texture.h"

namespace
{
    // Relative to the track's height, so a taller slider gets a proportionally chunkier
    // handle rather than a thin sliver.
    constexpr float HANDLE_ASPECT = 0.6f;
}

Engine::UISlider::UISlider() = default;
Engine::UISlider::~UISlider() = default;

void Engine::UISlider::SetRange(const float minimum, const float maximum)
{
    _min = minimum;

    // A zero-width range would divide by zero in GetNormalised and make every position
    // on the track mean the same value.
    _max = maximum > minimum ? maximum : minimum + 1.0f;

    SetValue(_value);
}

void Engine::UISlider::SetValue(const float value)
{
    const float clamped = std::clamp(value, _min, _max);
    if (clamped == _value) return;

    _value = clamped;

    if (_onChanged)
        _onChanged(_value);
}

float Engine::UISlider::GetNormalised() const
{
    return (_value - _min) / (_max - _min);
}

float Engine::UISlider::HandleWidth() const
{
    return _rect.size.y * HANDLE_ASPECT * 2.0f;
}

bool Engine::UISlider::HitTest(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) const
{
    if (!IsVisible()) return false;

    // Widened by half a handle at each end. At value 0 or 1 the handle hangs half
    // outside the track, and testing the track alone would leave the visible half of it
    // ungrabbable.
    const glm::vec2 topLeft = _rect.ResolvePosition(canvasSize);
    const float pad = HandleWidth() * 0.5f;

    return canvasPoint.x >= topLeft.x - pad && canvasPoint.x <= topLeft.x + _rect.size.x + pad
        && canvasPoint.y >= topLeft.y - pad && canvasPoint.y <= topLeft.y + _rect.size.y + pad;
}

void Engine::UISlider::ApplyFromPointer(const glm::vec2& canvasPoint, const glm::vec2& canvasSize)
{
    if (_rect.size.x <= 0.0f) return;

    const glm::vec2 topLeft = _rect.ResolvePosition(canvasSize);
    const float normalised = std::clamp((canvasPoint.x - topLeft.x) / _rect.size.x, 0.0f, 1.0f);

    SetValue(_min + normalised * (_max - _min));
}

bool Engine::UISlider::OnPointerPressed(const glm::vec2& canvasPoint, const glm::vec2& canvasSize)
{
    if (!IsVisible()) return false;

    _dragging = true;

    // Jumps to where the track was clicked rather than requiring the handle itself to be
    // grabbed first. Clicking a slider and having nothing happen is the more surprising
    // of the two behaviours.
    ApplyFromPointer(canvasPoint, canvasSize);

    return true;
}

void Engine::UISlider::OnPointerReleased(const glm::vec2& canvasPoint, const glm::vec2& canvasSize)
{
    _dragging = false;
}

void Engine::UISlider::OnPointerMoved(const glm::vec2& canvasPoint, const glm::vec2& canvasSize)
{
    _hovered = HitTest(canvasPoint, canvasSize);

    // Deliberately not gated on the pointer still being over the slider. Dragging a
    // slider and sliding off it vertically must keep tracking horizontally, which is how
    // every other slider behaves; the canvas ends the drag by delivering the release.
    if (_dragging)
        ApplyFromPointer(canvasPoint, canvasSize);
}

void Engine::UISlider::Draw(const glm::mat4& projection, const glm::vec2& canvasSize)
{
    if (!IsVisible()) return;

    const glm::vec2 topLeft = _rect.ResolvePosition(canvasSize);
    const float normalised = GetNormalised();

    DrawQuad(projection, topLeft, _rect.size, _tint * _trackColor, _opacity);

    if (normalised > 0.0f)
        DrawQuad(projection, topLeft, glm::vec2(_rect.size.x * normalised, _rect.size.y),
                 _tint * _fillColor, _opacity);

    const float handleWidth = HandleWidth();
    const float handleHeight = _rect.size.y * 2.0f;

    // Centred on the value, and on the track vertically -- so it overhangs top and
    // bottom and reads as a grip rather than as part of the bar.
    const glm::vec2 handlePos(
        topLeft.x + _rect.size.x * normalised - handleWidth * 0.5f,
        topLeft.y - (handleHeight - _rect.size.y) * 0.5f);

    const glm::vec3 handle = _hovered || _dragging ? _handleColor : _handleColor * 0.75f;

    DrawQuad(projection, handlePos, glm::vec2(handleWidth, handleHeight), _tint * handle, _opacity);
}
