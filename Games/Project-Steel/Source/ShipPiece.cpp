#include "ShipPiece.h"

void ShipPiece::Start()
{
    _collider = gameObject->GetComponent<Engine::Collider>();
    _collider->SetEnabled(false);

    
}

void ShipPiece::Place()
{
    _collider->SetEnabled(true);
}
