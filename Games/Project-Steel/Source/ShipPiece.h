#pragma once
#include "Engine/Components/ComponentUtils.h"

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
    bool Place() const;

private:
    void CreateShip() const;
    std::vector<glm::vec2> GetGridPoints(glm::vec2 position, glm::vec2 size) const;
    void OnOverlapBegin(Engine::GameObject* go);
    void OnOverlapEnd(Engine::GameObject* go);
    
private:
    Engine::Collider* _collider;
    bool _preview = true;
    ShipPiece* _attachedToShip = nullptr;

public:
    void SetAttachedToShip(ShipPiece* piece) { _attachedToShip = piece; };
    
};

REGISTER_COMPONENT(ShipPiece)
