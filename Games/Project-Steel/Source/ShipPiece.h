#pragma once
#include "SteelTypes.h"
#include "Engine/Components/ComponentUtils.h"

namespace NetCode { class InputByteStream; }

namespace Engine
{
    class Collider;
}

class ShipPiece final : public Engine::Component
{
public:
    ShipPiece() = default;
    
    void Start() override;
    void Update(float deltaTime) override;

    glm::vec2 GetNearestSnapLocation(glm::vec2 mouse, const ShipPiece* other) const;

    // --- building ---
    //
    // Placement is a *request*, never an act. A client's preview piece is local
    // scenery; the piece that actually exists is spawned by the server, replicated
    // back, and is a different object. Going through the server even when this machine
    // is the server is deliberate -- one code path, one set of rules, no second copy of
    // the validation that can drift.
    bool RequestPlacement() const;

    // Authority only. Validates the request and builds the piece, or returns nullptr
    // if the build is refused. Everything but `builder` came off the wire.
    static ShipPiece* PlaceAuthoritative(EShipPieceType type, glm::vec2 position, float rotation,
                                         uint32_t attachToNetworkID, Engine::GameObject* builder);

    void SetPieceType(const EShipPieceType type) { _type = type; }
    EShipPieceType GetPieceType() const { return _type; }
    bool IsPreview() const { return _preview; }

private:
    // Turns a freshly spawned piece into a real one: collision responses, ship
    // parenting, and replication. Authority only.
    void FinalizePlacement(ShipPiece* attachTo);

    void CreateShip() const;
    std::vector<glm::vec2> GetGridPoints(glm::vec2 position, glm::vec2 size, const ShipPiece* other = nullptr) const;
    void OnOverlapBegin(Engine::GameObject* go);
    void OnOverlapEnd(Engine::GameObject* go);
    
private:
    Engine::Collider* _collider = nullptr;
    bool _preview = true;
    EShipPieceType _type = EShipPieceType::Hallway;
    ShipPiece* _attachedToShip = nullptr;

    // How far from their pawn a player may build. Enforced on the server, because the
    // position in a placement request is whatever the client chose to send.
    static constexpr float MAX_BUILD_DISTANCE = 600.0f;

public:
    void SetAttachedToShip(ShipPiece* piece) { _attachedToShip = piece; }
    
};

REGISTER_COMPONENT(ShipPiece)
