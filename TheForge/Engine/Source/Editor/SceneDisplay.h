#pragma once
#include <glm/glm.hpp>

#include "Engine/Rendering/UIWindow.h"

namespace Editor
{
    class SceneDisplay final : public Engine::UIWindow
    {
    public:

        SceneDisplay();
        void Render() override;

        void ClickAndDrag();

    private:

        // `imageOrigin` is the screen position of the scene image's top-left corner,
        // captured immediately before it is submitted -- the image is not at the window
        // origin, and with multi-viewport ImGui neither is in SDL window coordinates.
        void HandleSelection(glm::vec2 imageOrigin, glm::vec2 imageSize);

    private:

        // Where the left button went down, so a click can be told from a camera pan --
        // both are the same button in the same window.
        glm::vec2 _pressPosition = glm::vec2(0.0f);
        glm::vec2 _lastClickPosition = glm::vec2(0.0f);

        // A click that moved less than this is a selection, not a drag. Also the radius
        // within which a second click counts as "the same spot" for cycling.
        static constexpr float CLICK_TOLERANCE_PIXELS = 4.0f;
    };
}
