#pragma once
#include <memory>
#include <vector>

#include "UIElement.h"

namespace Engine
{
    // A collection of UI elements sharing one coordinate space and one projection.
    //
    // The projection is built here from the canvas size and is entirely independent of
    // the scene camera. That is the point of this whole layer: CameraHelper's projection
    // is identity when no Camera component exists, and a main menu -- which has no player
    // and therefore no camera -- would draw everything into raw NDC.
    class UICanvas
    {
    public:
        UICanvas();

        // Takes ownership. Returns a borrowed pointer for the caller to configure and
        // keep; it stays valid until the canvas is cleared or the element is removed.
        template <typename T, typename... Args>
        T* Add(Args&&... args)
        {
            static_assert(std::is_base_of_v<UIElement, T>, "UICanvas holds UIElements");

            auto element = std::make_unique<T>(std::forward<Args>(args)...);
            T* raw = element.get();
            _elements.push_back(std::move(element));
            return raw;
        }

        // For elements owned elsewhere -- a component adapter holds its own, because the
        // GameObject's lifetime decides when it goes, not the canvas's.
        void AddBorrowed(UIElement* element);
        void RemoveBorrowed(UIElement* element);

        void Clear();

        bool IsEmpty() const { return _elements.empty() && _borrowed.empty(); }

        // Routes hover and click to the front-most element that wants them, then ticks
        // every element. `pointer` is in canvas units; `pointerDown` is the raw button
        // state this frame.
        //
        // Returns true if the pointer is over an element that takes input, so the caller
        // can stop the same click reaching the world underneath.
        bool Update(float deltaTime, const glm::vec2& pointer, bool pointerDown);

        // Draws back-to-front, in the order elements were added.
        void Render();

        // Defaults to Engine::ReferenceResolution. Overriding it is how a screen opts
        // into being laid out at a different scale.
        void SetCanvasSize(const glm::vec2& size) { _canvasSize = size; }
        const glm::vec2& GetCanvasSize() const { return _canvasSize; }

        glm::mat4 GetProjection() const;

        // True while the pointer sits over something interactive. Read by the input
        // plumbing so a click on a button does not also fire in the world.
        bool IsPointerOverUI() const { return _pointerOverUI; }

        // Blocks input from reaching anything behind this canvas even where there is no
        // element under the cursor. What a modal screen -- a pause menu, a loading
        // screen -- wants.
        void SetBlocksInput(const bool blocks) { _blocksInput = blocks; }
        bool BlocksInput() const { return _blocksInput; }

    private:
        // Front-most first. Elements are drawn in insertion order, so the last one added
        // is on top and must therefore be the first one offered a click.
        std::vector<UIElement*> BuildHitOrder() const;

    private:
        std::vector<std::unique_ptr<UIElement>> _elements;
        std::vector<UIElement*> _borrowed;

        glm::vec2 _canvasSize;
        bool _pointerOverUI = false;
        bool _blocksInput = false;

        // Edge detection lives here rather than being read from the InputManager,
        // because a canvas has to know whether the press *started* on it: a drag that
        // began in the world and released over a button must not click that button.
        bool _pointerWasDown = false;
        UIElement* _pressedElement = nullptr;
    };
}
