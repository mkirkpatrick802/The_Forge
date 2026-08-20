#include "GameSession.h"

#include <cstdlib>

#include "CommandRegistry.h"
#include "EngineManager.h"
#include "GameEngine.h"
#include "JsonKeywords.h"
#include "LaunchOptions.h"
#include "Level.h"
#include "LevelManager.h"
#include "System.h"
#include "UI/LoadingScreen.h"
#include "UI/PauseScreen.h"
#include "UI/SettingsScreen.h"
#include "UI/UIActionRegistry.h"
#include "UI/UIRoot.h"
#include "../../../Netcode/Source/NetworkManager.h"

float Engine::GameSession::_connectElapsed = -1.0f;
bool Engine::GameSession::_connectPending = false;

namespace
{
    // The loading screen currently up, or nullptr. Borrowed -- UIRoot owns every screen
    // on its stack -- and only ever read on the frame it was looked up, because a pop
    // frees it.
    Engine::LoadingScreen* CurrentLoadingScreen()
    {
        Engine::UIScreen* top = Engine::UIRoot::GetTop();
        if (top == nullptr || top->GetName() != Engine::LoadingScreen::SCREEN_NAME) return nullptr;

        return static_cast<Engine::LoadingScreen*>(top);
    }
}

std::string Engine::GameSession::GetMainMenuLevel()
{
    const auto configured = GetEngineManager().GetConfigData(DEFAULTS_FILE, JsonKeywords::Config::MAIN_MENU_LEVEL);

    return configured.is_string() ? configured.get<std::string>() : std::string();
}

bool Engine::GameSession::IsInMainMenu()
{
    const std::string menu = GetMainMenuLevel();
    if (menu.empty()) return false;

    Level* level = LevelManager::GetCurrentLevel();
    if (level == nullptr) return false;

    return level->GetName() == menu;
}

void Engine::GameSession::ConnectToServer(const std::string& address, const uint16_t port)
{
    // Nothing here makes sense without a window: a dedicated server hosts rather than
    // joins, and a headless client has nowhere to show a loading screen.
    if (GetLaunchOptions().headless)
    {
        DEBUG_LOG("Session: refusing to connect from a headless process -- it has no menu to have asked.")
        return;
    }

    std::string target = address;
    uint16_t targetPort = port;

    if (target.empty())
    {
        if (const auto configured = GetEngineManager().GetConfigData(DEFAULTS_FILE, JsonKeywords::Config::SERVER_ADDRESS);
            configured.is_string())
            target = configured.get<std::string>();
    }

    if (targetPort == 0)
    {
        if (const auto configured = GetEngineManager().GetConfigData(DEFAULTS_FILE, JsonKeywords::Config::SERVER_PORT);
            configured.is_number_unsigned())
            targetPort = configured.get<uint16_t>();
    }

    // Before StartNetCode, which is what reads the role and picks the transport.
    SetSessionTarget(ENetRole::ENR_Client, target, targetPort);

    const LaunchOptions& options = GetLaunchOptions();

    auto loading = std::make_unique<LoadingScreen>();
    loading->SetStatus("Connecting to " + options.serverAddress + ":" + std::to_string(options.port));
    loading->SetOnBack([] { ReturnToMenu(); });

    // Replace, not push: there is no going back to the menu with the Back button while a
    // connection is in flight, and leaving the menu underneath would have it draw again
    // for a frame if the loading screen ever popped.
    UIRoot::Replace(std::move(loading));

    // Queued, not started here. This function is almost always reached from a button's
    // click callback, and the canvas is still iterating its elements at that point --
    // the Replace above will destroy the very button whose callback this is. Doing
    // anything further now, including forcing the screen stack to flush, would run the
    // rest of this on freed memory.
    //
    // Update picks it up on the next frame, by which point UIRoot has swapped the
    // screens and the loading screen's log mirror is installed -- so the first line of
    // the handshake lands on the screen rather than being lost.
    _connectPending = true;
    _connectElapsed = 0.0f;
}

void Engine::GameSession::ReturnToMenu()
{
    const std::string menu = GetMainMenuLevel();
    if (menu.empty())
    {
        DEBUG_LOG("Session: no main menu level is configured -- nothing to return to.")
        return;
    }

    _connectElapsed = -1.0f;
    _connectPending = false;

    NetCode::GetNetworkManager().ShutdownNetCode();

    UIRoot::PopAll();

    // OpenLevel and not StartCurrentLevel: the menu's game mode is what puts the menu
    // screen up, so it has to be started -- but the "press Play" path would also reload
    // from disk, turn the editor off and adopt a camera, none of which applies here.
    LevelManager::OpenLevel(LEVEL_PATH + menu + ".json");

    DEBUG_LOG("Session: returned to the main menu.")
}

void Engine::GameSession::RegisterCommands()
{
    CommandRegistry::RegisterCommand("/connect", [](const std::string& args)
    {
        // The argument is an address, optionally with a port: "/connect 10.0.0.4:7777".
        std::string address = args;
        uint16_t port = 0;

        if (const size_t colon = address.find(':'); colon != std::string::npos)
        {
            port = static_cast<uint16_t>(std::strtoul(address.substr(colon + 1).c_str(), nullptr, 10));
            address = address.substr(0, colon);
        }

        ConnectToServer(address, port);
    });

    CommandRegistry::RegisterCommand("/menu", [](const std::string&) { ReturnToMenu(); });

    // The engine-level actions an authored button can be pointed at. A game registers
    // its own on top; see UIActionRegistry for why a level names behaviour by string
    // rather than reaching for code directly.
    UIActionRegistry::Register("Play", [] { ConnectToServer(); });
    UIActionRegistry::Register("Settings", [] { UIRoot::Push(std::make_unique<SettingsScreen>()); });
    UIActionRegistry::Register("Main Menu", [] { ReturnToMenu(); });
    UIActionRegistry::Register("Quit", [] { APPLICATION_CLOSING = true; });
}

void Engine::GameSession::Update(const float deltaTime)
{
    if (_connectPending)
    {
        _connectPending = false;

        const LaunchOptions& options = GetLaunchOptions();
        DEBUG_LOG("Session: connecting to %s:%u.", options.serverAddress.c_str(), options.port)

        NetCode::GetNetworkManager().StartNetCode();

        // Nothing to watch yet this frame. Whether the loading screen is on the stack
        // depends on whether UIRoot has flushed since ConnectToServer queued it, and
        // that varies with who called it -- a button callback runs inside UIRoot::Update
        // and gets flushed on the way out, a console command does not.
        return;
    }

    if (_connectElapsed < 0.0f) return;

    _connectElapsed += deltaTime;

    if (NetCode::GetNetworkManager().IsPlaying())
    {
        // The world has arrived and been loaded -- LevelManager::LoadLevel(stream) ran
        // from inside ProcessPacket, so there is a level to show.
        _connectElapsed = -1.0f;

        if (CurrentLoadingScreen() != nullptr)
            UIRoot::Pop();

        DEBUG_LOG("Session: world received -- entering the game.")
        return;
    }

    if (_connectElapsed < CONNECT_TIMEOUT_SECONDS) return;

    _connectElapsed = -1.0f;

    // Said out loud rather than left as a screen that reads "Connecting" for ever. A
    // refused or unreachable server is the common case in development, and it is
    // indistinguishable from a hang without this.
    //
    // The screen is looked up here and not earlier on purpose. An earlier version
    // treated "the loading screen is not on top" as "give up watching" and cleared the
    // timer -- which fired on the very frame a connection was requested, whenever the
    // caller had not already flushed the screen stack. The result was a loading screen
    // that stayed up for ever because nothing was left watching for the world.
    if (LoadingScreen* loading = CurrentLoadingScreen(); loading != nullptr && !loading->HasFailed())
    {
        loading->SetFailed("No answer from " + GetLaunchOptions().serverAddress +
                           ". The server may be down, or refusing this build.");
    }

    DEBUG_LOG("Session: gave up connecting after %.0f seconds.", CONNECT_TIMEOUT_SECONDS)
}
