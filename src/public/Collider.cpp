#include "Collider.h"

#include <sstream>

#include "GameObject.h"
#include "GameObjectManager.h"

void Collider::LoadData(const json& data)
{
    const float sideLength = data["Size"];
    size = Vector2D(sideLength);

    sweep = data["Sweep Collisions"];
}

void Collider::BeginPlay()
{
	Component::BeginPlay();

    _objectManager = GameObjectManager::GetInstance();
}

void Collider::Update(const float deltaTime)
{
    Component::Update(deltaTime);


    if (!sweep) return;
    if (CheckCollision())
        gameObject->Destroy();
}

bool Collider::CheckCollision() const
{
	for (const auto go : _objectManager->GetCurrentGameObjects())
        if (go->GetComponent<Collider>() && go != gameObject && go != gameObject->owner)
        {
	        const Collider* other = go->GetComponent<Collider>();

            // Calculate the min and max points for both colliders
	        const glm::vec2 thisMin = gameObject->transform.position - (size * 0.5f);
	        const glm::vec2 thisMax = gameObject->transform.position + (size * 0.5f);

	        const glm::vec2 otherMin = other->gameObject->transform.position - (other->size * 0.5f);
	        const glm::vec2 otherMax = other->gameObject->transform.position + (other->size * 0.5f);

            // Check for overlap in both X and Y axes
            if (thisMin.x < otherMax.x && thisMax.x > otherMin.x &&
                thisMin.y < otherMax.y && thisMax.y > otherMin.y)
            {
                return true; // Collision detected
            }
        }

    return false;
}