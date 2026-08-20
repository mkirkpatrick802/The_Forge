#pragma once
#include <functional>

#include "UIImage.h"

namespace Engine
{
    // A draggable value between two bounds: a track, a fill showing how far along it is,
    // and a handle to grab.
    //
    // All three are drawn from this one element rather than being separate children,
    // because they are not independently positionable -- the fill and handle are entirely
    // derived from the value and the track's rect.
    class UISlider final : public UIImage
    {
    public:
        UISlider();
        ~UISlider() override;

        void Draw(const glm::mat4& projection, const glm::vec2& canvasSize) override;

        bool OnPointerPressed(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) override;
        void OnPointerReleased(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) override;
        void OnPointerMoved(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) override;

        // Hit testing is widened by the handle's half-width at each end, so the handle
        // stays grabbable when the value is at either extreme -- at 0 and 1 half of it
        // hangs outside the track.
        bool HitTest(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) const override;

        void SetRange(float minimum, float maximum);

        void SetValue(float value);
        float GetValue() const { return _value; }

        // 0..1 along the track.
        float GetNormalised() const;

        // Called whenever the value changes, including while dragging -- so a volume
        // slider is heard as it moves rather than only when it is let go.
        void SetOnChanged(std::function<void(float)> onChanged) { _onChanged = std::move(onChanged); }

        void SetTrackColor(const glm::vec3& color) { _trackColor = color; }
        void SetFillColor(const glm::vec3& color) { _fillColor = color; }
        void SetHandleColor(const glm::vec3& color) { _handleColor = color; }

    private:
        // Maps a pointer position to a value and applies it. Shared by the initial press
        // and the drag, so clicking anywhere on the track jumps there rather than doing
        // nothing until the handle itself is grabbed.
        void ApplyFromPointer(const glm::vec2& canvasPoint, const glm::vec2& canvasSize);

        float HandleWidth() const;

    private:
        float _value = 1.0f;
        float _min = 0.0f;
        float _max = 1.0f;

        std::function<void(float)> _onChanged;

        glm::vec3 _trackColor = glm::vec3(0.16f, 0.18f, 0.22f);
        glm::vec3 _fillColor = glm::vec3(0.4f, 0.75f, 1.0f);
        glm::vec3 _handleColor = glm::vec3(0.9f);

        bool _dragging = false;
        bool _hovered = false;
    };
}
