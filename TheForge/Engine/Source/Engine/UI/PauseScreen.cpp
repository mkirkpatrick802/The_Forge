#include "PauseScreen.h"

#include "SettingsScreen.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIRoot.h"
#include "UIText.h"
#include "Engine/GameSession.h"

namespace
{
    constexpr float BUTTON_WIDTH = 110.0f;
    constexpr float BUTTON_HEIGHT = 15.0f;
    constexpr float BUTTON_SPACING = 18.0f;
}

void Engine::PauseScreen::OnEnter()
{
    _canvas.Clear();

    // Blocks input even where there is no element under the cursor. Without this, a
    // click on the gap between two buttons would fall through to the world and fire
    // whatever is bound to it -- which, in a game about building, means placing a piece.
    _canvas.SetBlocksInput(true);

    const glm::vec2 canvasSize = _canvas.GetCanvasSize();

    // Translucent, so the game stays visible behind it. This is the element that proves
    // the UI layer composites over the scene rather than replacing it.
    const auto shade = _canvas.Add<UIImage>();
    shade->Rect().size = canvasSize;
    shade->SetTint(glm::vec3(0.02f, 0.03f, 0.05f));
    shade->SetOpacity(0.72f);

    const auto title = _canvas.Add<UIText>("PAUSED", 18);
    title->Rect().anchor = EUIAnchor::MiddleCenter;
    title->Rect().offset = glm::vec2(0.0f, -34.0f);
    title->Rect().size = glm::vec2(canvasSize.x, 18.0f);
    title->Rect().pivot = glm::vec2(0.5f, 1.0f);
    title->SetAlign(EUITextAlign::Center);

    int index = 0;
    const auto add = [&](const std::string& label, std::function<void()> onClick)
    {
        const auto button = _canvas.Add<UIButton>(label, std::move(onClick));
        button->Rect().anchor = EUIAnchor::MiddleCenter;
        button->Rect().pivot = glm::vec2(0.5f, 0.0f);
        button->Rect().offset = glm::vec2(0.0f, -12.0f + static_cast<float>(index) * BUTTON_SPACING);
        button->Rect().size = glm::vec2(BUTTON_WIDTH, BUTTON_HEIGHT);
        button->Label().SetFontSize(10);
        ++index;

        return button;
    };

    add("RESUME", [] { UIRoot::Pop(); });

    add("SETTINGS", [] { UIRoot::Push(std::make_unique<SettingsScreen>()); });

    // Leaves the server outright. Reloading a level under a live session is unsolved
    // here -- every network id a peer knows becomes invalid -- so quitting to the menu
    // shuts the session down rather than trying to keep the connection alive.
    add("QUIT TO MENU", [] { GameSession::ReturnToMenu(); });

    const auto hint = _canvas.Add<UIText>("esc to resume", 6);
    hint->Rect().anchor = EUIAnchor::BottomCenter;
    hint->Rect().offset = glm::vec2(0.0f, -6.0f);
    hint->Rect().size = glm::vec2(canvasSize.x, 6.0f);
    hint->Rect().pivot = glm::vec2(0.5f, 1.0f);
    hint->SetAlign(EUITextAlign::Center);
    hint->SetColor(glm::vec3(0.5f, 0.53f, 0.57f));
}
