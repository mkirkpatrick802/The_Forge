#include "LoadingScreen.h"

#include <algorithm>

#include "UIButton.h"
#include "UIImage.h"
#include "UIText.h"
#include "Engine/GameEngine.h"
#include "Engine/System.h"
#include "Engine/Time.h"
#include "Engine/Rendering/Renderer.h"

namespace
{
    // Lines kept in the feed. Enough to see the shape of a handshake without the screen
    // becoming a wall of text.
    constexpr size_t LOG_LINES = 10;

    // At most one present per display refresh. Vsync is on, so a present blocks until
    // the next one anyway -- reporting more often than this would make the loading
    // screen the slowest part of the load.
    constexpr uint64_t PRESENT_INTERVAL_MS = 16;

    constexpr float BAR_WIDTH = 200.0f;
    constexpr float BAR_HEIGHT = 4.0f;

    // Rough, and deliberately so. The feed is fixed-width Consolas, and a line longer
    // than the canvas would run off the right edge rather than wrapping -- there is no
    // wrapping in UIText, and a loading screen is not the place to introduce one.
    constexpr size_t LOG_MAX_CHARS = 92;
}

Engine::LoadingScreen::LoadingScreen()
    : UIScreen(SCREEN_NAME)
{
}

Engine::LoadingScreen::~LoadingScreen()
{
    // The destructor as well as OnExit. A screen destroyed without being popped -- which
    // is what UIRoot::CleanUp does at shutdown -- would otherwise leave a dangling
    // callback in a process-wide slot, pointing at freed memory.
    OnExit();
}

void Engine::LoadingScreen::OnEnter()
{
    _canvas.Clear();
    _canvas.SetBlocksInput(true);
    _logLabels.clear();

    const glm::vec2 canvasSize = _canvas.GetCanvasSize();

    // A solid backdrop rather than an image, so the screen works before any art exists
    // for it. Give the element a sprite and it becomes a full-bleed background instead.
    _background = _canvas.Add<UIImage>();
    _background->Rect().anchor = EUIAnchor::TopLeft;
    _background->Rect().size = canvasSize;
    _background->SetTint(glm::vec3(0.05f, 0.06f, 0.08f));

    _title = _canvas.Add<UIText>("LOADING", 18);
    _title->Rect().anchor = EUIAnchor::TopCenter;
    _title->Rect().offset = glm::vec2(0.0f, 26.0f);
    _title->Rect().size = glm::vec2(canvasSize.x, 18.0f);
    _title->Rect().pivot = glm::vec2(0.5f, 0.0f);
    _title->SetAlign(EUITextAlign::Center);
    _title->SetColor(glm::vec3(0.95f));

    _status = _canvas.Add<UIText>(_statusText, 9);
    _status->Rect().anchor = EUIAnchor::TopCenter;
    _status->Rect().offset = glm::vec2(0.0f, 52.0f);
    _status->Rect().size = glm::vec2(canvasSize.x - 40.0f, 9.0f);
    _status->Rect().pivot = glm::vec2(0.5f, 0.0f);
    _status->SetAlign(EUITextAlign::Center);
    _status->SetColor(glm::vec3(0.7f, 0.78f, 0.85f));

    // Wrapped, because this line is not authored: it carries a server address and, on
    // failure, a whole sentence explaining what went wrong. Unwrapped, the failure
    // message ran off both edges of the screen -- so the one line a stuck player most
    // needs to read was the one they could not.
    _status->SetWrap(true);

    _progressTrack = _canvas.Add<UIImage>();
    _progressTrack->Rect().anchor = EUIAnchor::TopCenter;
    _progressTrack->Rect().offset = glm::vec2(0.0f, 68.0f);
    _progressTrack->Rect().size = glm::vec2(BAR_WIDTH, BAR_HEIGHT);
    _progressTrack->Rect().pivot = glm::vec2(0.5f, 0.0f);
    _progressTrack->SetTint(glm::vec3(0.16f, 0.18f, 0.22f));

    // Added after the track so it draws over it, and anchored from the track's left edge
    // rather than centred -- a centred fill would grow in both directions.
    _progressFill = _canvas.Add<UIImage>();
    _progressFill->Rect().anchor = EUIAnchor::TopCenter;
    _progressFill->Rect().offset = glm::vec2(-BAR_WIDTH * 0.5f, 68.0f);
    _progressFill->Rect().size = glm::vec2(0.0f, BAR_HEIGHT);
    _progressFill->SetTint(glm::vec3(0.4f, 0.75f, 1.0f));

    // A fixed set of labels reused as a scrolling window, rather than one label created
    // and destroyed per line as the feed moves. Every label shares a font through
    // UIResources, so holding ten of them costs ten pointers.
    for (size_t i = 0; i < LOG_LINES; ++i)
    {
        const auto line = _canvas.Add<UIText>("", 5);
        line->Rect().anchor = EUIAnchor::BottomLeft;
        line->Rect().offset = glm::vec2(8.0f, -6.0f - static_cast<float>(LOG_LINES - 1 - i) * 7.0f);
        line->Rect().pivot = glm::vec2(0.0f, 1.0f);
        line->Rect().size = glm::vec2(canvasSize.x - 16.0f, 5.0f);
        line->SetColor(glm::vec3(0.55f, 0.6f, 0.66f));
        _logLabels.push_back(line);
    }

    // Everything the engine logs from here on shows on the screen. The connect,
    // authenticate and world-state sequence already goes through DEBUG_LOG, so this is
    // what makes the "logs / netcode messages / states" this screen is for appear
    // without netcode having to know a loading screen exists.
    //
    // The previous mirror is kept and put back in OnExit rather than cleared: the slot
    // is shared with the editor terminal and the chat window.
    _previousLogMirror = System::GetLogMirror();
    _mirrorInstalled = true;
    System::SetLogMirror([this](const std::string& line) { AppendLog(line); });

    // Registered last, so nothing can report into a half-built screen.
    SetLoadProgressSink(this);
}

void Engine::LoadingScreen::OnExit()
{
    // Guarded on identity, not merely on being non-null: a nested load would have
    // replaced the sink, and clearing it blind would silence whoever owns it now.
    if (GetLoadProgressSink() == this)
        SetLoadProgressSink(nullptr);

    if (_mirrorInstalled)
    {
        System::SetLogMirror(_previousLogMirror);
        _previousLogMirror = nullptr;
        _mirrorInstalled = false;
    }
}

void Engine::LoadingScreen::SetStatus(const std::string& status)
{
    _statusText = status;
    _stageText.clear();
}

void Engine::LoadingScreen::SetFailed(const std::string& reason)
{
    _failed = true;
    _statusText = reason;
    _stageText.clear();
    _fraction = -1.0f;

    if (_title != nullptr)
    {
        _title->SetText("COULD NOT CONNECT");
        _title->SetColor(glm::vec3(1.0f, 0.5f, 0.4f));
    }

    if (_status != nullptr)
        _status->SetColor(glm::vec3(0.95f, 0.75f, 0.7f));

    // A bar on a failed screen suggests something is still happening.
    if (_progressTrack != nullptr) _progressTrack->SetVisible(false);
    if (_progressFill != nullptr) _progressFill->SetVisible(false);

    // A dead end otherwise. Without a way out, a failed connection leaves the player
    // looking at an apology with nothing to click, and the only remaining option is to
    // kill the process.
    if (_backButton == nullptr && _onBack)
    {
        _backButton = _canvas.Add<UIButton>("BACK", [this] { if (_onBack) _onBack(); });
        _backButton->Rect().anchor = EUIAnchor::TopCenter;
        _backButton->Rect().offset = glm::vec2(0.0f, 76.0f);
        _backButton->Rect().pivot = glm::vec2(0.5f, 0.0f);
        _backButton->Rect().size = glm::vec2(70.0f, 14.0f);
        _backButton->Label().SetFontSize(9);

        // The screen blocks input as a whole -- there is nothing behind it to protect --
        // but the button itself still has to be reachable.
        _canvas.SetBlocksInput(true);
    }
}

void Engine::LoadingScreen::AppendLog(const std::string& line)
{
    if (line.empty()) return;

    std::string trimmed = line;

    // Log lines arrive with their newline attached, which would draw as a missing glyph
    // rather than as a break.
    while (!trimmed.empty() && (trimmed.back() == '\n' || trimmed.back() == '\r'))
        trimmed.pop_back();

    if (trimmed.empty()) return;

    if (trimmed.size() > LOG_MAX_CHARS)
        trimmed = trimmed.substr(0, LOG_MAX_CHARS - 3) + "...";

    _logLines.push_back(std::move(trimmed));

    while (_logLines.size() > LOG_LINES)
        _logLines.pop_front();

    RefreshLogLabels();
}

void Engine::LoadingScreen::RefreshLogLabels()
{
    // The feed is bottom-aligned, so a partly filled feed leaves the top rows blank and
    // the newest line always sits on the same row.
    const size_t count = std::min(_logLines.size(), _logLabels.size());
    const size_t firstUsed = _logLabels.size() - count;

    for (size_t i = 0; i < _logLabels.size(); ++i)
        _logLabels[i]->SetText(i < firstUsed ? std::string() : _logLines[i - firstUsed]);
}

void Engine::LoadingScreen::Update(const float deltaTime)
{
    _elapsed += deltaTime;

    if (_status != nullptr)
    {
        std::string text = _stageText.empty() ? _statusText : _stageText;

        // An animated ellipsis on anything still in progress. Without it an
        // indeterminate stage is a still image, and a still image is what a hang looks
        // like.
        if (!_failed)
        {
            const int dots = static_cast<int>(_elapsed * 2.0f) % 4;
            text.append(static_cast<size_t>(dots), '.');
        }

        _status->SetText(text);
    }

    if (_progressFill != nullptr && !_failed)
    {
        // A negative fraction means the stage has no measurable progress, so the bar
        // shows nothing rather than pretending to sit at zero.
        const float clamped = std::clamp(_fraction, 0.0f, 1.0f);
        _progressFill->Rect().size.x = _fraction < 0.0f ? 0.0f : BAR_WIDTH * clamped;
    }
}

void Engine::LoadingScreen::Report(const float fraction, const std::string& stage)
{
    _fraction = fraction;
    _stageText = stage;

    PresentIfDue();
}

void Engine::LoadingScreen::PresentIfDue()
{
    const uint64_t now = Time::GetTicks();
    if (now - _lastPresentTicks < PRESENT_INTERVAL_MS) return;

    _lastPresentTicks = now;

    // Ticked by hand, because the main loop is not running: Report is called from inside
    // a blocking load. A fixed step rather than a real delta, since Time::SetDeltaTime
    // belongs to the game loop and the only thing this drives is the ellipsis.
    Update(PRESENT_INTERVAL_MS / 1000.0f);

    GetRenderer().PresentUIOnly();
}
