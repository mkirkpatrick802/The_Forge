#pragma once
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../../Netcode/Source/NetActions.h"

namespace Engine
{
    class Component;
    class GameObject;
    class PlayerStart;

    // The rules of the match, and the only place that decides them.
    //
    // Authority-only by construction: a level that arrives over the wire is built
    // without a game mode at all, because a client must never decide where players
    // spawn or which team they are on. Every hook below therefore runs on the server.
    class GameModeBase
    {
    public:
        GameModeBase();
        virtual ~GameModeBase() = default;

        virtual void Start();

        // Ticked on the authority once the level is running. The base does nothing;
        // a mode with phases -- a lobby waiting to become a match -- drives them here.
        virtual void Update(float deltaTime) {}

        virtual GameObject* SpawnPlayer(uint64_t playerID);

        // A peer has connected and said who it is. Distinct from the pawn being
        // spawned, which happens on connection, before anyone knows who is driving:
        // anything keyed on identity -- team, saved preferences, a scoreboard entry --
        // belongs here rather than in SpawnPlayer.
        virtual void OnPlayerIdentified(NetCode::PeerID peer, const NetCode::PlayerIdentity& identity) {}

        virtual void OnPlayerLeft(NetCode::PeerID peer, const NetCode::PlayerIdentity& identity) {}

    protected:
        std::vector<PlayerStart*> _playerStarts;
        std::string _playerPrefab = "Assets/Prefabs/Player.prefab";
    };

    // Lets a game supply its own rules without the engine knowing what they are.
    //
    // A level names its mode in JSON; the level loader asks here for one by that name
    // and falls back to the base if the name is unknown or absent, so a level authored
    // before a mode existed still loads.
    class GameModeRegistry
    {
    public:
        using Factory = std::function<std::unique_ptr<GameModeBase>()>;

        static void Register(const std::string& name, Factory factory);
        static std::unique_ptr<GameModeBase> Create(const std::string& name);

    private:
        static std::unordered_map<std::string, Factory>& Factories();
    };
}

// Registers a game mode under its own class name at static-init time, matching how
// REGISTER_COMPONENT works elsewhere in the engine.
#define REGISTER_GAME_MODE(Type)                                                        \
    namespace                                                                            \
    {                                                                                    \
        const bool Type##_registered = []                                                \
        {                                                                                \
            Engine::GameModeRegistry::Register(#Type, [] { return std::make_unique<Type>(); }); \
            return true;                                                                 \
        }();                                                                             \
    }
