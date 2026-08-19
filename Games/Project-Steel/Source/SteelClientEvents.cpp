#include "SteelTypes.h"
#include "Engine/System.h"

// Client-side handlers for the events the server broadcasts.
//
// Registered at static-init time rather than from the game mode, because a client has
// no game mode -- a level that arrives over the wire is built without one, deliberately.
// The registry is a function-local static, so registering this early is safe.
//
// These are where match UI will hang off once there is any: for now they put the state
// somewhere visible so the flow can be watched from a log.
namespace
{
    const bool steelClientEventsRegistered = []
    {
        NetCode::NetActionRegistry::RegisterEvent(SteelEvent::MatchStateChanged,
            [](NetCode::InputByteStream& payload)
            {
                uint8_t rawState;
                float secondsRemaining;
                payload.Read(rawState);
                payload.Read(secondsRemaining);

                const auto state = static_cast<EMatchState>(rawState);
                if (secondsRemaining > 0.0f)
                    DEBUG_LOG("Match: %s (%.0fs).", MatchStateName(state), secondsRemaining)
                else
                    DEBUG_LOG("Match: %s.", MatchStateName(state))
            });

        NetCode::NetActionRegistry::RegisterEvent(SteelEvent::TeamAssigned,
            [](NetCode::InputByteStream& payload)
            {
                uint8_t rawTeam;
                bool isMercenary;
                payload.Read(rawTeam);
                payload.Read(isMercenary);

                const auto team = static_cast<ETeam>(rawTeam);
                DEBUG_LOG("Team: you are %s%s.", TeamName(team), isMercenary ? " (mercenary -- the sides were uneven)" : "")
            });

        return true;
    }();
}
