//
// Created by mKirkpatrick on 1/30/2024.
//

#include "GameObject.h"

#include <sstream>

#include "Component.h"
#include "SpriteRenderer.h"
#include "Renderer.h"

GameObject::GameObject() {
    _transform = Transform();

    settings.name = &_name;
    settings.isReplicated = &_isReplicated;
    settings.position = &_transform.position;
}

GameObject::GameObject(float x, float y): settings(), _isReplicated(false)
{
	_transform = Transform(x, y);
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
    if(mousePos.x >= _transform.position.x - size.x / 2 && mousePos.x <= _transform.position.x + size.x / 2)
        if(mousePos.y >= _transform.position.y - size.y / 2 && mousePos.y <= _transform.position.y + size.y / 2)
            return true;

    return false;
}

std::vector<Component *> GameObject::GetAttachedComponents() {
    return _attachedComponents;
}

Vector2D GameObject::GetPosition() const
{
    return _transform.position;
}

string GameObject::GetPositionString() const
{
    std::stringstream ss;
    ss << _transform.position.x << " " << _transform.position.y;
    return ss.str();
}

void GameObject::SetPosition(Vector2D newPosition)
{
    _transform.position = newPosition;
}

GameObject::~GameObject() {
    //printf("%s Deleted \n", _name.c_str());
}
