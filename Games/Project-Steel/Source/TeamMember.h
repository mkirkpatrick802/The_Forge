#pragma once
#include "SteelTypes.h"
#include "Engine/Components/ComponentUtils.h"

// Which side the thing this is attached to belongs to.
//
// A component rather than a field on PlayerController because teams are Project
// Steel's idea, not the engine's -- and because it is not only players that have a
// side. A base, a ship, and a turret all need one, and none of them is a pawn.
//
// Added at spawn by SteelGameMode rather than authored into the Player prefab, so
// the team can be decided by the rules at the moment a player joins rather than
// baked into an asset.
class TeamMember final : public Engine::Component
{
public:
    TeamMember();

    void Write(NetCode::OutputByteStream& stream) const override;
    void Read(NetCode::InputByteStream& stream) override;

    void DrawDetails() override;

    nlohmann::json Serialize() override;
    void Deserialize(const json& data) override;

    ETeam GetTeam() const { return _team; }

    // Authority only. Marks the owning object dirty so the change actually goes out;
    // a team that changes without replicating is how a client ends up shooting its
    // own side.
    void SetTeam(ETeam team);

private:
    ETeam _team = ETeam::None;
};

REGISTER_COMPONENT(TeamMember)
