#pragma once
#include "Engine/GameModeBase.h"

// The rules of the main menu, which are that there are none.
//
// A game mode rather than a free-floating screen because that is how this engine gives
// a level something that ticks: Level::UpdateGameMode is the only per-frame gameplay
// hook the loop calls that is not a component. Naming this mode in MainMenu.json is
// therefore what makes the menu appear when that level opens -- including when the
// player quits back to it from a match.
//
// Everything it does is a refusal of what the base mode does:
//
//   Start        does NOT bring the netcode up. The base does, because the base is what
//                a playable level uses -- and a menu that opened a transport would be
//                hosting a session nobody asked for, before the player had chosen a
//                server. GameSession::ConnectToServer starts it when Play is pressed.
//
//   SpawnPlayer  returns nothing. There is no world here to put a pawn in, and no
//                camera -- which is exactly why the UI layer draws through its own
//                projection rather than a camera's.
class MainMenuGameMode final : public Engine::GameModeBase
{
public:
    void Start() override;
    Engine::GameObject* SpawnPlayer(uint64_t playerID) override;
};

REGISTER_GAME_MODE(MainMenuGameMode)
