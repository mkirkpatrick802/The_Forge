#include "Collider.h"

#include "Client.h"
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

void Collider::RegisterHitCallback(const HitCallbackFunction& function)
{
    callback = function;
}

void Collider::TriggerHitCallback(GameObject* hit)
{
    if (callback)
        callback(hit);
}

void Collider::Update(const float deltaTime)
{
    Component::Update(deltaTime);


    if (!sweep) return;
    CheckCollision();
}

bool Collider::CheckCollision()
{
	for (const auto go : _objectManager->GetCurrentGameObjects())
        if (go->GetComponent<Collider>() && go != gameObject && go != gameObject->owner)
        {
	        const Collider* other = go->GetComponent<Collider>();

            // Calculate the min and max points for both colliders
	        const glm::vec2 thisMin = gameObject->transform.position - size;
	        const glm::vec2 thisMax = gameObject->transform.position + size;

	        const glm::vec2 otherMin = other->gameObject->transform.position - other->size;
	        const glm::vec2 otherMax = other->gameObject->transform.position + other->size;

            // Check for overlap in both X and Y axes
            if (thisMin.x < otherMax.x && thisMax.x > otherMin.x &&
                thisMin.y < otherMax.y && thisMax.y > otherMin.y)
            {
                TriggerHitCallback(go);
                return true; // Collision detected
            }
        }

    return false;
}
