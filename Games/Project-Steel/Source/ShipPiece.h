#pragma once
#include "Engine/Collisions/Collider.h"
#include "Engine/Components/ComponentUtils.h"

class ShipPiece final : public Engine::Component
{
public:
    ShipPiece() = default;

    void Start() override;
    void Place();
    
private:
    Engine::Collider* _collider;
    bool _preview = true;
    
};

REGISTER_COMPONENT(ShipPiece)
