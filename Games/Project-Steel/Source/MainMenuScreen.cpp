#include "MainMenuScreen.h"

#include "Engine/GameSession.h"
#include "Engine/System.h"
#include "Engine/UI/SettingsScreen.h"
#include "Engine/UI/UIButton.h"
#include "Engine/UI/UIRoot.h"
#include "Engine/UI/UIImage.h"
#include "Engine/UI/UIText.h"

using namespace Engine;

namespace
{
    constexpr float BUTTON_WIDTH = 96.0f;
    constexpr float BUTTON_HEIGHT = 15.0f;
    constexpr float BUTTON_SPACING = 18.0f;

    // Measured down from the middle of the canvas, so the stack stays centred whatever
    // the window is doing.
    constexpr float FIRST_BUTTON_Y = 14.0f;

    UIButton* AddMenuButton(UICanvas& canvas, const std::string& label, const int index, std::function<void()> onClick)
    {
        const auto button = canvas.Add<UIButton>(label, std::move(onClick));
        button->Rect().anchor = EUIAnchor::MiddleCenter;
        button->Rect().pivot = glm::vec2(0.5f, 0.0f);
        button->Rect().offset = glm::vec2(0.0f, FIRST_BUTTON_Y + static_cast<float>(index) * BUTTON_SPACING);
        button->Rect().size = glm::vec2(BUTTON_WIDTH, BUTTON_HEIGHT);
        button->Label().SetFontSize(10);

        return button;
    }
}

void MainMenuScreen::OnEnter()
{
    _canvas.Clear();

    // Nothing behind the menu should be clickable, including the empty world the menu
    // level technically still is.
    _canvas.SetBlocksInput(true);

    const glm::vec2 canvasSize = _canvas.GetCanvasSize();

    // A solid field rather than artwork, so the menu stands up before anyone has drawn a
    // background for it. Giving this element a sprite is all it takes to change that.
    const auto background = _canvas.Add<UIImage>();
    background->Rect().size = canvasSize;
    background->SetTint(glm::vec3(0.06f, 0.07f, 0.10f));

    const auto logo = _canvas.Add<UIImage>("Assets/Engine Assets/Sprites/logo.png");
    logo->Rect().anchor = EUIAnchor::MiddleCenter;
    logo->Rect().pivot = glm::vec2(0.5f, 1.0f);
    logo->Rect().offset = glm::vec2(0.0f, -34.0f);
    logo->SizeToSprite();

    const auto title = _canvas.Add<UIText>("PROJECT STEEL", 20);
    title->Rect().anchor = EUIAnchor::MiddleCenter;
    title->Rect().pivot = glm::vec2(0.5f, 1.0f);
    title->Rect().offset = glm::vec2(0.0f, -8.0f);
    title->Rect().size = glm::vec2(canvasSize.x, 20.0f);
    title->SetAlign(EUITextAlign::Center);
    title->SetColor(glm::vec3(0.95f, 0.93f, 0.88f));

    // Play is a *join*, not a host. The dedicated server is the authority and it has no
    // menu of its own -- it comes up hosting and everyone else joins it.
    AddMenuButton(_canvas, "PLAY", 0, [] { GameSession::ConnectToServer(); });

    AddMenuButton(_canvas, "SETTINGS", 1, []
    {
        UIRoot::Push(std::make_unique<SettingsScreen>());
    });

    AddMenuButton(_canvas, "QUIT", 2, []
    {
        // The same flag the window's close button sets, so quitting from the menu and
        // quitting from the title bar shut down along exactly one path.
        APPLICATION_CLOSING = true;
    });

    const auto hint = _canvas.Add<UIText>("dedicated server -- address in Config/defaults.json", 6);
    hint->Rect().anchor = EUIAnchor::BottomCenter;
    hint->Rect().pivot = glm::vec2(0.5f, 1.0f);
    hint->Rect().offset = glm::vec2(0.0f, -6.0f);
    hint->Rect().size = glm::vec2(canvasSize.x, 6.0f);
    hint->SetAlign(EUITextAlign::Center);
    hint->SetColor(glm::vec3(0.45f, 0.48f, 0.52f));
}


