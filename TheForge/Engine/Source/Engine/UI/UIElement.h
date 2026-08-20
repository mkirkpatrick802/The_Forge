#pragma once
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>

namespace Engine
{
    // Where a rect's position is measured from.
    //
    // Authoring in reference-resolution units alone is not enough: a button pinned to
    // the bottom-right of a 320x180 canvas has to stay there, and saying "at (310, 170)"
    // only works while the canvas is exactly that size. The anchor names the corner the
    // offset is measured from, so the same element lands correctly whatever the canvas
    // turns out to be.
    enum class EUIAnchor : uint8_t
    {
        TopLeft = 0, TopCenter, TopRight,
        MiddleLeft, MiddleCenter, MiddleRight,
        BottomLeft, BottomCenter, BottomRight,
    };

    const char* UIAnchorName(EUIAnchor anchor);

    // A rectangle on the UI canvas, in reference-resolution units (see
    // Engine::ReferenceResolution -- 320x180, the same space the scene framebuffer is
    // drawn in). Kept in those units rather than pixels so the UI scales with the pixel
    // art instead of drifting away from it as the window resizes.
    struct UIRect
    {
        glm::vec2 offset = glm::vec2(0.0f);  // from the anchor point
        glm::vec2 size = glm::vec2(0.0f);
        EUIAnchor anchor = EUIAnchor::TopLeft;
        glm::vec2 pivot = glm::vec2(0.0f);   // 0..1 within the element's own box

        // The element's top-left corner in canvas units.
        glm::vec2 ResolvePosition(const glm::vec2& canvasSize) const;

        bool Contains(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) const;
    };

    // A thing drawn on a UICanvas.
    //
    // Deliberately NOT a Component and not an IRenderable. Both of those require a
    // GameObject in a Level, and the loading screen has to exist when there is no level
    // at all -- that is the whole reason this layer is separate from the render list.
    // Level-authored UI reaches this through a component adapter instead, so the
    // dependency runs that way round rather than this one.
    class UIElement
    {
    public:
        UIElement() = default;
        virtual ~UIElement() = default;

        UIElement(const UIElement&) = delete;
        UIElement& operator=(const UIElement&) = delete;

        // Drawn with the canvas's own orthographic projection, which is built from the
        // canvas size and owes nothing to the scene camera. A UI element must never call
        // CameraHelper::GetProjectionMatrix() -- that returns identity when no Camera
        // component exists, which is exactly the case on a menu with no player in it.
        virtual void Draw(const glm::mat4& projection, const glm::vec2& canvasSize) = 0;

        virtual void Update(float deltaTime) {}

        // Creates GPU-side resources if they do not exist yet. Called from the canvas
        // immediately before drawing and never at load, mirroring the contract
        // IRenderable::EnsureResourcesResident defines: a headless build never draws, so
        // it never needs a GL context. Overrides must call the base first.
        virtual void EnsureResourcesResident() {}

        // Drops GPU resources so they are rebuilt on the next draw. Call after changing
        // anything they are derived from -- a sprite path, a font size.
        virtual void InvalidateResources() {}

        // Screen-space hit test, in canvas units. Only meaningful for elements that take
        // input; the base rect test is what buttons and sliders build on.
        virtual bool HitTest(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) const
        {
            return IsVisible() && _rect.Contains(canvasPoint, canvasSize);
        }

        // True if this element swallowed the click, so nothing behind it sees the same
        // press. The canvas walks elements front-to-back and stops at the first one that
        // says yes.
        virtual bool OnPointerPressed(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) { return false; }
        virtual void OnPointerReleased(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) {}
        virtual void OnPointerMoved(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) {}

    public:
        UIRect& Rect() { return _rect; }
        const UIRect& Rect() const { return _rect; }

        bool IsVisible() const { return _visible; }
        void SetVisible(const bool visible) { _visible = visible; }

        const glm::vec3& GetTint() const { return _tint; }
        void SetTint(const glm::vec3& tint) { _tint = tint; }

        float GetOpacity() const { return _opacity; }
        void SetOpacity(const float opacity) { _opacity = opacity; }

        const std::string& GetName() const { return _name; }
        void SetName(const std::string& name) { _name = name; }

    protected:
        UIRect _rect;
        glm::vec3 _tint = glm::vec3(1.0f);
        float _opacity = 1.0f;
        bool _visible = true;
        std::string _name;
    };
}
