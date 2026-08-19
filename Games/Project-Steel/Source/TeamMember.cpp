#include "TeamMember.h"

#include "imgui.h"
#include "Engine/GameObject.h"

TeamMember::TeamMember()
{
    // Team changes at runtime -- assignment, balancing, a base changing hands -- so it
    // belongs in a per-tick delta rather than only in the full object record.
    isReplicated = true;
}

void TeamMember::Write(NetCode::OutputByteStream& stream) const
{
    stream.Write(_team);
}

void TeamMember::Read(NetCode::InputByteStream& stream)
{
    stream.Read(_team);
}

void TeamMember::SetTeam(const ETeam team)
{
    if (_team == team) return;

    _team = team;

    if (gameObject != nullptr)
        gameObject->MarkDirty();
}

nlohmann::json TeamMember::Serialize()
{
    nlohmann::json data = Component::Serialize();
    data["Team"] = static_cast<uint8_t>(_team);
    return data;
}

void TeamMember::Deserialize(const json& data)
{
    if (data.contains("Team"))
        _team = static_cast<ETeam>(data["Team"].get<uint8_t>());
}

void TeamMember::DrawDetails()
{
    ImGui::Text("Team: %s", TeamName(_team));
}
