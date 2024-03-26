//
// Created by mKirkpatrick on 1/30/2024.
//

#ifndef THE_FORGE_GAMEOBJECT_H
#define THE_FORGE_GAMEOBJECT_H

#include <vector>
#include <string>
#include "Transform.h"
#include "UIStructs.h"

class Component;

class GameObject {

public:

    friend class GameObjectManager;

public:

    GameObject();
    GameObject(float x, float y);

    ~GameObject();

    void ObjectCreated();

    Component* AddComponent(Component* component);

    template <typename T>
    T* GetComponent();

    Vector2D GetPosition();
    string GetPositionString();

    void SetPosition(Vector2D newPosition);

    bool IsClicked(Vector2D mousePos);

    std::vector<Component*> GetAttachedComponents();

private:
public:

    GameObjectSettings settings;
    Transform _transform;

private:

    std::string _name;

    std::vector<Component*> _attachedComponents;
};

template <typename T>
T* GameObject::GetComponent() {
    for (auto component : _attachedComponents) {
        if (T* childComp = dynamic_cast<T*>(component)) {
            return childComp;
        }
    }

    return nullptr;
}

#endif //THE_FORGE_GAMEOBJECT_H
