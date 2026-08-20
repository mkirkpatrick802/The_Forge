#include "ShipPiece.h"

#include "Engine/Level.h"
#include "Engine/LevelManager.h"
#include "Engine/System.h"
#include "Engine/Collisions/CollisionManager.h"
#include "Engine/Components/RectangleCollider.h"
#include "Engine/Rendering/DebugRenderer.h"
#include "LinkingContext.h"
#include "NetworkManager.h"

using namespace Engine;

std::string SHIP_MANAGER_PREFAB = "Assets/Prefabs/Ship Manager.prefab";

void ShipPiece::Start()
{
    // A preview is local scenery on one machine and must never go on the wire. A piece
    // the server built is already past FinalizePlacement by the time this runs, so the
    // flag is only cleared for pieces that really are previews.
    if (_preview)
        gameObject->isReplicated = false;

    _collider = gameObject->GetComponent<Collider>();

    // A ship piece authored without a collider used to crash here, on the first frame,
    // on every machine that loaded the level -- including a headless server, which
    // takes the whole match down. Snapping and overlap need a collider, so a piece
    // without one simply does not take part; that is a content problem to report, not
    // a reason to die.
    if (_collider == nullptr)
    {
        DEBUG_LOG("ShipPiece on '%s' has no Collider; it cannot be snapped to or built on.", gameObject->GetName().c_str())
        return;
    }

    if (gameObject->GetParent() == nullptr)
    {
        _collider->SetCollisionResponseToAllObjects(ECollisionResponse::ECR_Overlap);
        _collider->SetCollisionResponseByObject(ECollisionObjectType::ECOT_Player, ECollisionResponse::ECR_Ignore);
    }

    _collider->OnOverlapBegin.Bind(this, &ShipPiece::OnOverlapBegin);
    _collider->OnOverlapEnd.Bind(this, &ShipPiece::OnOverlapEnd);
}

void ShipPiece::Update(float deltaTime)
{
    
}

glm::vec2 ShipPiece::GetNearestSnapLocation(glm::vec2 mouse, const ShipPiece* other) const
{
    // Both sides of a snap need bounds. A piece that survived Start without a collider
    // reaches here otherwise, and the cast below would be reading through null.
    if (_collider == nullptr || other == nullptr || other->_collider == nullptr)
        return gameObject->GetWorldPosition();

    // The grid is built around the *collider*, not the object's origin, so a piece
    // whose collider is offset still snaps along the edges of its actual footprint.
    glm::vec2 position = _collider->GetCenter();
    float rotation = gameObject->GetWorldRotation(); // You might need this if you're rotating grid points
    glm::vec2 size = ((RectangleCollider*)_collider)->GetSize();
    glm::vec2 otherSize = other->gameObject->GetComponent<RectangleCollider>()->GetSize();
    
    const std::vector<glm::vec2> gridPoints = GetGridPoints(position, size, other);
    if (gridPoints.empty()) return glm::vec2(0.0f);
    
    // Find the closest snap point to the other piece (mouse position)
    glm::vec2 closestSnapPoint = gridPoints[0];
    float minDistance = std::numeric_limits<float>::max();

    for (const auto& gridPoint : gridPoints)
    {
        const float distance = glm::distance(gridPoint, mouse);
        if (distance < minDistance)
        {
            minDistance = distance;
            closestSnapPoint = gridPoint;
        }
    }

    return closestSnapPoint;
}

std::vector<glm::vec2> ShipPiece::GetGridPoints(glm::vec2 position, glm::vec2 size, const ShipPiece* other) const
{
    std::vector<glm::vec2> gridPoints;
    glm::vec2 halfSize = size * 0.5f;
    glm::vec2 min = position - halfSize - 16.f;
    glm::vec2 max = position + halfSize + 16.f;

    for (float x = min.x; x <= max.x; x += 16.f)
    {
        for (float y = min.y; y <= max.y; y += 16.f)
        {
            bool isBorderX = (x == min.x || x == max.x);
            bool isBorderY = (y == min.y || y == max.y);

            // Exclude corners
            if ((isBorderX || isBorderY) && !(isBorderX && isBorderY))
            {
                auto point = glm::vec2(x, y);
                std::vector<Collider*> collidingObjects;
                std::vector<Collider*> ignoreColliders;
                if (other)
                    ignoreColliders.emplace_back(other->_collider);
                
                if (!GetCollisionManager().CheckCollisions(point, collidingObjects, ECollisionObjectType::ECOT_Default | ECollisionObjectType::ECOT_Walkable, ignoreColliders))
                    gridPoints.emplace_back(point);
            }
        }
    }

    for (auto point : gridPoints)
        GetDebugRenderer().DrawRectangle(point, glm::vec2(8), glm::vec3(.5,1,.5));
    
    return gridPoints;
}

bool ShipPiece::RequestPlacement() const
{
    if (gameObject == nullptr) return false;

    // A local overlap check first, so an obviously bad placement costs nothing and the
    // player gets an immediate no. It is a courtesy, not a guarantee -- the server
    // repeats it, because this one runs on a machine the server does not control.
    std::vector<Collider*> others;
    if (_collider != nullptr && GetCollisionManager().CheckCollisions(_collider, others))
        return false;

    const uint32_t attachID = _attachedToShip != nullptr
        ? NetCode::GetLinkingContext().GetNetworkID(_attachedToShip->gameObject, false)
        : NULL_ID;

    NetCode::OutputByteStream payload;
    payload.Write(static_cast<uint8_t>(_type));
    payload.Write(gameObject->GetWorldPosition());
    payload.Write(gameObject->GetWorldRotation());
    payload.Write(attachID);

    // The host runs its own request through the same handler a remote client's would
    // reach, rather than calling the build code directly -- one set of rules, not two.
    if (NetCode::GetNetworkManager().HasWorldAuthority())
        return NetCode::GetNetworkManager().DispatchLocalRequest(SteelRequest::PlaceShipPiece, payload);

    return NetCode::GetNetworkManager().SendRequest(SteelRequest::PlaceShipPiece, payload);
}

ShipPiece* ShipPiece::PlaceAuthoritative(const EShipPieceType type, const glm::vec2 position, const float rotation,
                                         const uint32_t attachToNetworkID, Engine::GameObject* builder)
{
    const char* prefab = GetShipPiecePrefab(type);
    if (prefab == nullptr) return nullptr;

    // Range is checked against the builder's pawn, not against anything in the request.
    // Without this a client can build anywhere on the map, including inside the other
    // team's base.
    if (builder != nullptr)
    {
        const glm::vec2 offset = position - builder->GetWorldPosition();
        if (offset.x * offset.x + offset.y * offset.y > MAX_BUILD_DISTANCE * MAX_BUILD_DISTANCE)
            return nullptr;
    }

    // Resolved through the linking context rather than trusted as a pointer: an id that
    // names something which is not a ship piece is a request to refuse, not to follow.
    ShipPiece* attachTo = nullptr;
    if (attachToNetworkID != NULL_ID)
    {
        if (const auto attachGo = NetCode::GetLinkingContext().GetGameObject(attachToNetworkID))
            attachTo = attachGo->GetComponent<ShipPiece>();

        if (attachTo == nullptr) return nullptr;
    }

    Level* level = LevelManager::GetCurrentLevel();
    if (level == nullptr) return nullptr;

    GameObject* go = level->SpawnNewGameObject(prefab, position);
    if (go == nullptr) return nullptr;

    go->SetRotation(rotation);

    const auto piece = go->GetComponent<ShipPiece>();
    const auto collider = go->GetComponent<Collider>();
    if (piece == nullptr || collider == nullptr)
    {
        level->RemoveGameObject(go, false);
        return nullptr;
    }

    piece->_type = type;
    piece->_collider = collider;

    // The overlap test the client already did, repeated where it counts. Spawn-then-test
    // rather than test-then-spawn because the collider is what gets tested, and it does
    // not exist until the object does.
    if (std::vector<Collider*> blocking; GetCollisionManager().CheckCollisions(collider, blocking))
    {
        level->RemoveGameObject(go, false);
        return nullptr;
    }

    piece->FinalizePlacement(attachTo);
    return piece;
}

void ShipPiece::FinalizePlacement(ShipPiece* attachTo)
{
    _preview = false;
    _attachedToShip = attachTo;

    gameObject->isReplicated = true;

    _collider->SetCollisionResponseByObject(
        ECollisionObjectType::ECOT_Player,
        _collider->GetCollisionProfile().type == ECollisionObjectType::ECOT_Walkable
            ? ECollisionResponse::ECR_Overlap
            : ECollisionResponse::ECR_Block
    );

    if (_attachedToShip)
    {
        if (const auto manager = _attachedToShip->gameObject->GetParent())
            manager->AddChild(gameObject, true);
    }

    CreateShip();
    gameObject->MarkDirty();
}

void ShipPiece::CreateShip() const
{
    if (gameObject->GetParent() == nullptr)
    {
        const auto manager = LevelManager::GetCurrentLevel()->SpawnNewGameObject(SHIP_MANAGER_PREFAB, gameObject->GetWorldPosition());
        manager->AddChild(gameObject);
        gameObject->SetPosition(glm::vec2(0.0f, 0.0f));
    }
}

void ShipPiece::OnOverlapBegin(Engine::GameObject* go)
{
    
}

void ShipPiece::OnOverlapEnd(Engine::GameObject* go)
{
    
}
