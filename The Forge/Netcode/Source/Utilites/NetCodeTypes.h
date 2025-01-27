#pragma once
#include <cstdint>

namespace NetCode
{
    // Enum for Lobby Types
    enum class LobbyType : uint8_t
    {
        Private = 0,
        FriendsOnly,
        Public,
        Invisible
    };
}
