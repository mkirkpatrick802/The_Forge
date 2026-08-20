#pragma once
#include <string>

namespace Engine
{
    // Something that wants to hear how a level load is getting on.
    //
    // Level loading is one blocking call on the main thread -- nothing in the engine
    // except the headless server console uses a thread, and the component pools, the
    // registry and the linking context are all global mutable state, so making the
    // loader concurrent is a much larger change than it looks. The loading screen
    // therefore gets its frames the cooperative way: the loader calls Report as it goes,
    // and Report is what puts a frame on screen.
    //
    // Report may be called very often -- once per batch of game objects -- so an
    // implementation is responsible for rate-limiting whatever it does. See
    // LoadingScreen::Report, which presents at most one frame per display refresh.
    class LoadProgress
    {
    public:
        virtual ~LoadProgress() = default;

        // fraction is 0..1 and stage is what is happening now, in words a player could
        // read. A stage with no measurable progress passes a negative fraction, which
        // means "indeterminate" rather than "zero" -- a bar stuck at 0% and a bar that
        // does not apply look identical otherwise.
        virtual void Report(float fraction, const std::string& stage) = 0;
    };

    // The progress sink for the load currently running, or nullptr if nothing is
    // listening.
    //
    // A global rather than a parameter threaded through every caller: LoadLevel is
    // reached from the editor, from a console command, from the game mode and from an
    // arriving world-state packet, and none of those has any business knowing whether a
    // loading screen happens to be up.
    void SetLoadProgressSink(LoadProgress* sink);
    LoadProgress* GetLoadProgressSink();

    // Convenience for the loader: reports only if something is listening.
    void ReportLoadProgress(float fraction, const std::string& stage);
}
