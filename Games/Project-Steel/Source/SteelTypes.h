#pragma once
#include <cstdint>

#include "NetActions.h"

// Wire vocabulary for Project Steel. Every value here travels between machines, so
// none of them may be renumbered once a build is out -- an old client and a new
// server disagreeing about what a number means is silent misbehaviour rather than a
// clean failure.

// Which side a player is on. Red and Blue are the two factions from the concept;
// None is a player who has not been assigned yet, not a third team.
enum class ETeam : uint8_t
{
    None = 0,
    Red,
    Blue,
};

inline const char* TeamName(const ETeam team)
{
    switch (team)
    {
    case ETeam::Red:  return "Red";
    case ETeam::Blue: return "Blue";
    default:          return "Unassigned";
    }
}

inline ETeam OtherTeam(const ETeam team)
{
    return team == ETeam::Red ? ETeam::Blue : ETeam::Red;
}

// Where the match is in its life.
//
// The server never stops running, so Lobby is a real playable state rather than a
// waiting room: players fly, mine and build in it, and bases can be destroyed. What
// Live adds is that the result is recorded.
enum class EMatchState : uint8_t
{
    Lobby = 0,      // sandbox; populated by whoever is around
    Countdown,      // enough players, or an admin said go
    Live,           // map has been reset, result counts
    PostMatch,      // winner shown, briefly, before returning to Lobby
};

inline const char* MatchStateName(const EMatchState state)
{
    switch (state)
    {
    case EMatchState::Lobby:     return "Lobby";
    case EMatchState::Countdown: return "Countdown";
    case EMatchState::Live:      return "Live";
    case EMatchState::PostMatch: return "PostMatch";
    default:                     return "?";
    }
}

// The buildable pieces.
//
// Deliberately an id rather than a prefab path. A client asks for a piece by number
// and the server looks the path up in its own table -- a client-supplied path would
// be a client telling the server which file to open, which is not a thing a server
// should let a client do. Adding a piece means adding a row here and in the table in
// SteelActions.cpp, and nothing else changes.
enum class EShipPieceType : uint8_t
{
    Hallway = 0,

    Max
};

// The prefab for a piece type, or nullptr if the id is not a real piece. Callers must
// treat nullptr as "the client sent nonsense" rather than as a missing asset.
const char* GetShipPiecePrefab(EShipPieceType type);

namespace SteelRequest
{
    enum : NetCode::NetActionID
    {
        // Build a piece. Payload: piece type, world position, rotation, and the
        // network id of the piece to attach to (0 for a new, free-floating ship).
        PlaceShipPiece = NetCode::GAME_ACTION_BASE + 0,

        // Which faction the player would like to be. Honoured when it does not make
        // the teams uneven -- see SteelGameMode::AssignTeam.
        SetTeamPreference = NetCode::GAME_ACTION_BASE + 1,
    };
}

namespace SteelEvent
{
    enum : NetCode::NetActionID
    {
        // Payload: match state, seconds remaining in it (0 when open-ended).
        MatchStateChanged = NetCode::GAME_ACTION_BASE + 0,

        // Payload: team, and whether the player got the side they asked for. A
        // player put on the other side to even the numbers is a "mercenary" -- the
        // concept's word -- and the client is told so it can say so.
        TeamAssigned = NetCode::GAME_ACTION_BASE + 1,
    };
}
