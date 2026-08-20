#include "MainMenuGameMode.h"

#include "MainMenuScreen.h"
#include "Engine/LaunchOptions.h"
#include "Engine/System.h"
#include "Engine/UI/UIRoot.h"

void MainMenuGameMode::Start()
{
    // Deliberately not GameModeBase::Start(). That brings the netcode up, which is right
    // for a level someone is playing and wrong here: opening a transport on the menu
    // would have the process hosting or joining before the player had said which server
    // they wanted. GameSession::ConnectToServer does it when Play is pressed.

    // A headless process has no window and never gets here in normal use, but a level is
    // loadable by name from the console and the rule is that presentation subsystems are
    // absent on a server rather than merely idle.
    if (Engine::GetLaunchOptions().headless) return;

    // Replace rather than push. Arriving at the menu means whatever was on screen before
    // -- a loading screen that failed, a pause menu from the match just left -- is done.
    Engine::UIRoot::Replace(std::make_unique<MainMenuScreen>());

    DEBUG_LOG("Menu: main menu ready.")
}

Engine::GameObject* MainMenuGameMode::SpawnPlayer(uint64_t playerID)
{
    // No world to spawn into. Returning null rather than falling through to the base,
    // which would look for a PlayerStart, find none, and log about it every time.
    return nullptr;
}
