#pragma once
#include <cstdint>
#include <string>

namespace Engine
{
    // Getting from the main menu into a game, and back out again.
    //
    // Sits between the UI and the netcode so that neither has to know about the other: a
    // menu button says "join a server", and what that means -- set the role, put a
    // loading screen up, start the transport, watch for the world to arrive, give up if
    // it never does -- lives here.
    //
    // Client and standalone only. A dedicated server has no menu and no loading screen;
    // it comes up straight into its level and hosts.
    class GameSession
    {
    public:
        // Points the process at a server and puts the loading screen up. The handshake
        // itself then runs over ordinary frames -- the main loop keeps turning, the
        // network manager keeps ticking, and Update below watches for the end of it.
        //
        // An empty address falls back to whatever Config/defaults.json says.
        static void ConnectToServer(const std::string& address = "", uint16_t port = 0);

        // Shuts the session down and loads the menu level locally.
        //
        // Deliberately a full teardown rather than a level swap that keeps the
        // connection: reloading a level under a live session is unsolved here -- every
        // network id a peer knows becomes invalid, which is the same reason
        // SteelGameMode::ResetMap is still a stub. Leaving a server is leaving it.
        static void ReturnToMenu();

        // Ticked from the game loop. Drives the loading screen's lifetime: dismisses it
        // when the world arrives, and fails it when it does not.
        static void Update(float deltaTime);

        // The level named by Config/defaults.json as the menu, or empty if none is
        // configured. Empty means this build has no menu and boots into its default
        // level, which is what the editor and the dedicated server do.
        static std::string GetMainMenuLevel();

        // Whether the menu level is the one currently open. What a pause menu checks
        // before offering "Quit to Menu" -- there is nothing to quit to when you are
        // already there.
        static bool IsInMainMenu();

        // How long to wait for the world after asking to join, before giving up and
        // saying so on the loading screen.
        static constexpr float CONNECT_TIMEOUT_SECONDS = 20.0f;

        // Registers "/connect [address]" and "/menu". Called once at startup.
        //
        // The same two things the menu buttons do, reachable from the console -- which
        // is how you drive them from a build with no mouse, and how you point a client
        // at an address other than the configured one without editing config.
        static void RegisterCommands();

    private:
        // Seconds since ConnectToServer, or a negative number when no attempt is in
        // flight.
        static float _connectElapsed;

        // Set by ConnectToServer, acted on by Update one frame later.
        //
        // The transport must not be started from inside the button callback that asked
        // for it. ConnectToServer is reached from UIButton::OnPointerReleased, which the
        // canvas is in the middle of iterating -- and putting the loading screen up pops
        // the menu, which destroys that very button. Queueing means the screen swap
        // happens through UIRoot's ordinary deferred flush, with the callback long
        // returned.
        static bool _connectPending;
    };
}
