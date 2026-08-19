#include "SteelTypes.h"

const char* GetShipPiecePrefab(const EShipPieceType type)
{
    // The server's own table. A client names a piece by id and this is where that id
    // becomes a file, so there is no path anywhere on the wire and no way for a client
    // to point the server at a file of its choosing.
    switch (type)
    {
    case EShipPieceType::Hallway: return "Assets/Prefabs/Large Hallway Tile.prefab";

    default: return nullptr;
    }
}
