#pragma once
#include <deque>
#include <functional>
#include <string>
#include <vector>

#include "UIScreen.h"
#include "Engine/LoadProgress.h"

namespace Engine
{
    class UIButton;
    class UIImage;
    class UIText;

    // What the player looks at while a level loads and a session comes up.
    //
    // Fed from two places that have nothing to do with each other:
    //
    //   1. Level loading, which is synchronous. The main loop is not running while it
    //      happens, so this screen gets its frames by being called back from inside the
    //      loader -- see LoadProgress and Renderer::PresentUIOnly.
    //
    //   2. The network handshake, which happens over many ordinary frames while the
    //      loop keeps turning. Nothing special is needed for that; the screen simply
    //      stays up and reads NetworkManager's state each tick.
    //
    // The log feed is the engine's own DEBUG_LOG output, captured through
    // System::SetLogMirror. That is deliberate rather than a shortcut: the whole
    // connect, authenticate and world-state sequence already logs, so mirroring it is
    // how the screen shows what is actually happening rather than a fictional
    // progress bar.
    class LoadingScreen final : public UIScreen, public LoadProgress
    {
    public:
        static constexpr const char* SCREEN_NAME = "Loading";

        LoadingScreen();
        ~LoadingScreen() override;

        void OnEnter() override;
        void OnExit() override;
        void Update(float deltaTime) override;

        // Opaque, and it swallows input: there is nothing behind it worth showing and
        // nothing worth clicking.
        bool IsTransparent() const override { return false; }
        bool IsInputTransparent() const override { return false; }

        // LoadProgress. Called from inside a blocking load, possibly thousands of times,
        // and is what puts a frame on screen.
        void Report(float fraction, const std::string& stage) override;

        // What the screen says it is doing, when nothing more specific has been
        // reported. Set by whoever put the screen up.
        void SetStatus(const std::string& status);

        // Gives up and shows a message with no progress bar. Used when a connection
        // times out -- without it a failed join is a screen that says "Connecting" for
        // ever, which is indistinguishable from a hang.
        void SetFailed(const std::string& reason);
        bool HasFailed() const { return _failed; }

        // What the Back button on a failed screen does. Taken as a callback rather than
        // calling GameSession directly, so this stays a UI element that knows nothing
        // about sessions or netcode. No callback means no button.
        void SetOnBack(std::function<void()> onBack) { _onBack = std::move(onBack); }

        // Adds one line to the feed directly, for callers that want to say something
        // that did not go through DEBUG_LOG.
        void AppendLog(const std::string& line);

    private:
        void PresentIfDue();
        void RefreshLogLabels();

    private:
        UIButton* _backButton = nullptr;
        UIImage* _background = nullptr;
        UIImage* _progressTrack = nullptr;
        UIImage* _progressFill = nullptr;
        UIText* _title = nullptr;
        UIText* _status = nullptr;
        std::vector<UIText*> _logLabels;

        std::deque<std::string> _logLines;

        std::function<void()> _onBack;
        std::string _statusText = "Loading";
        std::string _stageText;
        float _fraction = -1.0f;
        bool _failed = false;

        // Drives the animated ellipsis, so an indeterminate stage still looks alive.
        float _elapsed = 0.0f;

        // Ticks at the last present, so Report can rate-limit itself. Vsync is on, so a
        // present costs a whole display refresh -- presenting on every report would make
        // the loading screen slower than the load.
        uint64_t _lastPresentTicks = 0;

        // Put back on the way out. The mirror is a single global slot shared with the
        // chat window and the editor terminal, so clearing it outright would silently
        // stop their output instead of restoring it.
        std::function<void(const std::string&)> _previousLogMirror;
        bool _mirrorInstalled = false;
    };
}
