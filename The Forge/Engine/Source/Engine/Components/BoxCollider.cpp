#include "BoxCollider.h"

#include "SpriteRenderer.h"
#include "Engine/JsonKeywords.h"

Engine::BoxCollider::BoxCollider(): size()
{
    
}

void Engine::BoxCollider::Start()
{
    if (const auto sprite = gameObject->GetComponent<SpriteRenderer>())
        size = sprite->GetSize();
}

bool Engine::BoxCollider::CheckCollision(const GameObject* go) const
{
    if (!go->GetComponent<BoxCollider>()) return false;
    
    return false;
}

bool Engine::BoxCollider::CheckCollision(glm::vec2 position) const
{
    const glm::vec2 myPos = gameObject->transform.position;
    const glm::vec2 halfSize = size * 0.5f; // Half size for centering

    return (position.x >= myPos.x - halfSize.x && position.x <= myPos.x + halfSize.x &&
            position.y >= myPos.y - halfSize.y && position.y <= myPos.y + halfSize.y);
}


void Engine::BoxCollider::Deserialize(const json& data)
{
    
}

nlohmann::json Engine::BoxCollider::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::COMPONENT_ID] = GetComponentRegistry().GetComponentID<BoxCollider>();
    return data;
}
