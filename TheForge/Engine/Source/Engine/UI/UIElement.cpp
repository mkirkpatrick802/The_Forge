#include "UIElement.h"

const char* Engine::UIAnchorName(const EUIAnchor anchor)
{
    switch (anchor)
    {
    case EUIAnchor::TopLeft:      return "Top Left";
    case EUIAnchor::TopCenter:    return "Top Center";
    case EUIAnchor::TopRight:     return "Top Right";
    case EUIAnchor::MiddleLeft:   return "Middle Left";
    case EUIAnchor::MiddleCenter: return "Middle Center";
    case EUIAnchor::MiddleRight:  return "Middle Right";
    case EUIAnchor::BottomLeft:   return "Bottom Left";
    case EUIAnchor::BottomCenter: return "Bottom Center";
    case EUIAnchor::BottomRight:  return "Bottom Right";
    default:                      return "?";
    }
}

namespace
{
    // The anchor's position on the canvas, as a fraction of the canvas in each axis.
    glm::vec2 AnchorFraction(const Engine::EUIAnchor anchor)
    {
        switch (anchor)
        {
        case Engine::EUIAnchor::TopLeft:      return {0.0f, 0.0f};
        case Engine::EUIAnchor::TopCenter:    return {0.5f, 0.0f};
        case Engine::EUIAnchor::TopRight:     return {1.0f, 0.0f};
        case Engine::EUIAnchor::MiddleLeft:   return {0.0f, 0.5f};
        case Engine::EUIAnchor::MiddleCenter: return {0.5f, 0.5f};
        case Engine::EUIAnchor::MiddleRight:  return {1.0f, 0.5f};
        case Engine::EUIAnchor::BottomLeft:   return {0.0f, 1.0f};
        case Engine::EUIAnchor::BottomCenter: return {0.5f, 1.0f};
        case Engine::EUIAnchor::BottomRight:  return {1.0f, 1.0f};
        default:                              return {0.0f, 0.0f};
        }
    }
}

glm::vec2 Engine::UIRect::ResolvePosition(const glm::vec2& canvasSize) const
{
    // Anchor point, then the authored offset, then back off by the pivot so a pivot of
    // (0.5, 0.5) centres the element on the point rather than hanging it below-right.
    const glm::vec2 anchorPoint = AnchorFraction(anchor) * canvasSize;
    return anchorPoint + offset - (pivot * size);
}

bool Engine::UIRect::Contains(const glm::vec2& canvasPoint, const glm::vec2& canvasSize) const
{
    const glm::vec2 topLeft = ResolvePosition(canvasSize);

    return canvasPoint.x >= topLeft.x && canvasPoint.x <= topLeft.x + size.x
        && canvasPoint.y >= topLeft.y && canvasPoint.y <= topLeft.y + size.y;
}
