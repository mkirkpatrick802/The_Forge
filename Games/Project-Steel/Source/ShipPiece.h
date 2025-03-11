#pragma once
#include "Engine/Components/ComponentUtils.h"

class ShipPiece final : public Engine::Component
{
public:
    ShipPiece() = default;

    void Start() override;
    void Place();
    
private:
    bool _preview = true;
    
};

REGISTER_COMPONENT(ShipPiece)
