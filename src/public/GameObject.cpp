//
// Created by mKirkpatrick on 1/30/2024.
//

#include "GameObject.h"

#include <chrono>
#include <sstream>
#include <thread>

#include "Client.h"
#include "Component.h"
#include "GameObjectManager.h"
#include "ObjectStateWriter.h"
#include "SpriteRenderer.h"
#include "OnDestroyedEvent.h"
#include "Projectile.h"
#include "RespawnManager.h"

GameObject::GameObject() {
    transform = Transform();

    settings.name = &name;
    settings.isReplicated = &isReplicated;
    settings.position = &transform.position;
}

GameObject::GameObject(float x, float y): settings(), isReplicated(false)
{
	transform = Transform(x, y);
}

Component* GameObject::AddComponent(Component* component)
{
    _attachedComponents.push_back(component);
    return component;
}

void GameObject::ObjectCreated()
{
	for (const auto component : _attachedComponents)
	{
        component->BeginPlay();
	}
}

bool GameObject::IsClicked(Vector2D mousePos) {

    Vector2D size = GetComponent<SpriteRenderer>()->GetSize();
    if(mousePos.x >= transform.position.x - size.x / 2 && mousePos.x <= transform.position.x + size.x / 2)
        if(mousePos.y >= transform.position.y - size.y / 2 && mousePos.y <= transform.position.y + size.y / 2)
            return true;

    return false;
}

std::vector<Component *> GameObject::GetAttachedComponents() {
    return _attachedComponents;
}

void GameObject::SetRotationWithVector(const Vector2D vector, float offset)
{
	const Vector2D direction = glm::normalize(vector);

    // Calculate the angle in radians
	const float angle_rad = std::atan2(direction.y, direction.x);

    // Convert the angle from radians to degrees
    float angle_deg = glm::degrees(angle_rad) + offset;

    // Adjust the angle to be between 0 and 360 degrees
    if (angle_deg < 0) {
        angle_deg += 360.0f;
    }

    transform.rotation = angle_deg;
}

void GameObject::Destroy()
{
    if(Client::IsHostClient())
    {
        ObjectStateWriter::DestroyObject(instanceID);

        DestroyRequestReceived();
    }
}

void GameObject::DestroyRequestReceived()
{
    if (_shouldRespawn)
        RespawnManager::GetInstance()->StartRespawnTimer(this, 5);

    for (auto component : _attachedComponents)
        component->OnDestroyed();

    GameObjectManager::GetInstance()->DestroyGameObject(this);
}

Vector2D GameObject::GetPosition() const
{
    return transform.position;
}

string GameObject::GetPositionString() const
{
    std::stringstream ss;
    ss << transform.position.x << " " << transform.position.y;
    return ss.str();
}

void GameObject::SetPosition(Vector2D newPosition)
{
    transform.position = newPosition;
}

GameObject::~GameObject()
{
    //printf("%s Deleted \n", _name.c_str());
}
