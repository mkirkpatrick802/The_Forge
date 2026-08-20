#include "UICanvas.h"

#include <algorithm>
#include <glm/ext/matrix_clip_space.hpp>

#include "Engine/Rendering/Renderer.h"

Engine::UICanvas::UICanvas()
    : _canvasSize(ReferenceResolution)
{
}

void Engine::UICanvas::AddBorrowed(UIElement* element)
{
    if (element == nullptr) return;
    if (std::ranges::find(_borrowed, element) != _borrowed.end()) return;

    _borrowed.push_back(element);
}

void Engine::UICanvas::RemoveBorrowed(UIElement* element)
{
    std::erase(_borrowed, element);

    // A press in progress on the element that just left would otherwise be delivered to
    // freed memory when the button came up.
    if (_pressedElement == element)
        _pressedElement = nullptr;
}

void Engine::UICanvas::Clear()
{
    _elements.clear();
    _borrowed.clear();
    _pressedElement = nullptr;
    _pointerOverUI = false;
}

glm::mat4 Engine::UICanvas::GetProjection() const
{
    // Y-down: (0,0) is the top-left corner, which is the convention the rest of the UI
    // layer is written to and the one Font's glyph quads are already wound for.
    return glm::ortho(0.0f, _canvasSize.x, _canvasSize.y, 0.0f, -1.0f, 1.0f);
}

std::vector<Engine::UIElement*> Engine::UICanvas::BuildHitOrder() const
{
    std::vector<UIElement*> order;
    order.reserve(_elements.size() + _borrowed.size());

    for (const auto& element : _elements)
        order.push_back(element.get());

    for (UIElement* element : _borrowed)
        order.push_back(element);

    // Drawing runs front-to-back in this order, so hit testing has to run the other way
    // -- otherwise a button underneath a panel would swallow the click that visibly
    // landed on the panel.
    std::ranges::reverse(order);

    return order;
}

bool Engine::UICanvas::Update(const float deltaTime, const glm::vec2& pointer, const bool pointerDown)
{
    const std::vector<UIElement*> hitOrder = BuildHitOrder();

    _pointerOverUI = false;

    UIElement* hovered = nullptr;
    for (UIElement* element : hitOrder)
    {
        if (element == nullptr || !element->IsVisible()) continue;

        if (element->HitTest(pointer, _canvasSize))
        {
            hovered = element;
            break;
        }
    }

    // Hover is told to everything, not only the element under the cursor: a button that
    // was hovered last frame and is not now has to hear about it, or it stays lit.
    for (UIElement* element : hitOrder)
    {
        if (element != nullptr)
            element->OnPointerMoved(pointer, _canvasSize);
    }

    if (pointerDown && !_pointerWasDown)
    {
        // Press. Offered front-most first, and the first element to claim it stops the
        // search -- that claim is also what makes the click exclusive to this canvas.
        for (UIElement* element : hitOrder)
        {
            if (element == nullptr || !element->IsVisible()) continue;

            if (element->HitTest(pointer, _canvasSize) && element->OnPointerPressed(pointer, _canvasSize))
            {
                _pressedElement = element;
                break;
            }
        }
    }
    else if (!pointerDown && _pointerWasDown)
    {
        // Release goes to whatever took the press, wherever the cursor now is. The
        // element decides whether a release outside its own bounds counts as a click --
        // dragging off a button and letting go is a cancel, and only the button knows
        // that.
        if (_pressedElement != nullptr)
            _pressedElement->OnPointerReleased(pointer, _canvasSize);

        _pressedElement = nullptr;
    }

    _pointerWasDown = pointerDown;
    _pointerOverUI = hovered != nullptr;

    for (const auto& element : _elements)
        element->Update(deltaTime);

    for (UIElement* element : _borrowed)
    {
        if (element != nullptr)
            element->Update(deltaTime);
    }

    return _blocksInput || _pointerOverUI;
}

void Engine::UICanvas::Render()
{
    const glm::mat4 projection = GetProjection();

    for (const auto& element : _elements)
    {
        if (!element->IsVisible()) continue;

        // Here and nowhere else. Textures, shaders and font atlases are created on first
        // draw rather than at load, so that nothing on this layer needs a GL context
        // until something is actually being put on screen.
        element->EnsureResourcesResident();
        element->Draw(projection, _canvasSize);
    }

    for (UIElement* element : _borrowed)
    {
        if (element == nullptr || !element->IsVisible()) continue;

        element->EnsureResourcesResident();
        element->Draw(projection, _canvasSize);
    }
}
