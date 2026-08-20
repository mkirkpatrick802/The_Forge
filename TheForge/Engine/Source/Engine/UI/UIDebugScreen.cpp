#include "UIDebugScreen.h"

#include "UIButton.h"
#include "UIImage.h"
#include "UIRoot.h"
#include "UIText.h"
#include "Engine/CommandRegistry.h"
#include "Engine/System.h"

namespace
{
    // Small enough to read the canvas by, at 320x180.
    constexpr float MARKER = 8.0f;

    void AddCornerMarker(Engine::UICanvas& canvas, const Engine::EUIAnchor anchor,
                         const glm::vec2& offset, const glm::vec2& pivot, const glm::vec3& tint)
    {
        const auto marker = canvas.Add<Engine::UIImage>("Assets/Engine Assets/Sprites/logo.png");
        marker->Rect().anchor = anchor;
        marker->Rect().offset = offset;
        marker->Rect().pivot = pivot;
        marker->Rect().size = glm::vec2(MARKER);
        marker->SetTint(tint);
    }
}

void Engine::UIDebugScreen::OnEnter()
{
    _canvas.Clear();

    // Corners. Each is pivoted into its own corner so the marker sits fully inside the
    // canvas -- a marker half off-screen would look like an anchoring bug rather than
    // the intended result.
    AddCornerMarker(_canvas, EUIAnchor::TopLeft,     { 2.0f,  2.0f}, {0.0f, 0.0f}, {1.0f, 0.3f, 0.3f});
    AddCornerMarker(_canvas, EUIAnchor::TopRight,    {-2.0f,  2.0f}, {1.0f, 0.0f}, {0.3f, 1.0f, 0.3f});
    AddCornerMarker(_canvas, EUIAnchor::BottomLeft,  { 2.0f, -2.0f}, {0.0f, 1.0f}, {0.3f, 0.5f, 1.0f});
    AddCornerMarker(_canvas, EUIAnchor::BottomRight, {-2.0f, -2.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 0.3f});

    // Centre, at its native 48x48 and pivoted on its own middle.
    const auto logo = _canvas.Add<UIImage>("Assets/Engine Assets/Sprites/logo.png");
    logo->Rect().anchor = EUIAnchor::MiddleCenter;
    logo->Rect().pivot = glm::vec2(0.5f);
    logo->SizeToSprite();

    const auto hello = _canvas.Add<UIText>("HELLO", 16);
    hello->Rect().anchor = EUIAnchor::TopLeft;
    hello->Rect().offset = glm::vec2(14.0f, 3.0f);
    hello->SetColor(glm::vec3(1.0f));

    // Right-aligned against a rect wider than the text, which is the case that catches
    // an alignment implemented as "move the anchor" rather than "offset within the box".
    const auto aligned = _canvas.Add<UIText>("RIGHT ALIGNED", 8);
    aligned->Rect().anchor = EUIAnchor::BottomRight;
    aligned->Rect().offset = glm::vec2(-14.0f, -12.0f);
    aligned->Rect().size = glm::vec2(120.0f, 8.0f);
    aligned->Rect().pivot = glm::vec2(1.0f, 0.0f);
    aligned->SetAlign(EUITextAlign::Right);
    aligned->SetColor(glm::vec3(0.6f, 0.9f, 1.0f));

    // Two buttons, because the interesting cases are the interaction between them:
    // hovering one must not light the other, and a click must reach exactly one.
    const auto click = _canvas.Add<UIButton>("CLICK ME", []
    {
        static int count = 0;
        DEBUG_LOG("UI: button clicked (%d).", ++count)
    });
    click->Rect().anchor = EUIAnchor::MiddleCenter;
    click->Rect().pivot = glm::vec2(0.5f, 0.0f);
    click->Rect().offset = glm::vec2(0.0f, 32.0f);
    click->Rect().size = glm::vec2(80.0f, 14.0f);
    click->Label().SetFontSize(9);

    // Disabled, to confirm a dimmed button neither lights on hover nor fires.
    const auto disabled = _canvas.Add<UIButton>("DISABLED", []
    {
        DEBUG_LOG("UI: a disabled button fired -- this should never print.")
    });
    disabled->Rect().anchor = EUIAnchor::MiddleCenter;
    disabled->Rect().pivot = glm::vec2(0.5f, 0.0f);
    disabled->Rect().offset = glm::vec2(0.0f, 50.0f);
    disabled->Rect().size = glm::vec2(80.0f, 14.0f);
    disabled->Label().SetFontSize(9);
    disabled->SetEnabled(false);

    const auto caption = _canvas.Add<UIText>("ui test card -- /ui again to close", 8);
    caption->Rect().anchor = EUIAnchor::BottomCenter;
    caption->Rect().offset = glm::vec2(0.0f, -4.0f);
    caption->Rect().pivot = glm::vec2(0.5f, 1.0f);
    caption->SetColor(glm::vec3(0.7f));
}

void Engine::UIDebugScreen::RegisterCommand()
{
    CommandRegistry::RegisterCommand("/ui", [](const std::string&)
    {
        // A toggle rather than a push, so the command can be typed twice without
        // stacking test cards on top of each other.
        //
        // Tested against the *top* screen and not merely "is it open anywhere": Pop
        // removes whatever is on top, so a test card buried under a pause menu would
        // otherwise close the pause menu instead.
        if (const UIScreen* top = UIRoot::GetTop(); top != nullptr && top->GetName() == "UI Debug")
        {
            UIRoot::Pop();
            DEBUG_LOG("UI: test card closed.")
            return;
        }

        if (UIRoot::IsOpen("UI Debug"))
        {
            DEBUG_LOG("UI: the test card is open but not on top. Close what is above it first.")
            return;
        }

        UIRoot::Push(std::make_unique<UIDebugScreen>());
        DEBUG_LOG("UI: test card opened. Expect four coloured corner markers, a centred logo and two labels.")
    });
}
