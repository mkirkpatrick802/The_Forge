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

    friend class ObjectCreator;

public:

    GameObject();
    GameObject(float x, float y);

    ~GameObject();

    void ObjectCreated();

    Component* AddComponent(Component* component);

    template <typename T>
    T* GetComponent() const;

    Vector2D GetPosition() const;
    string GetPositionString() const;

    void SetPosition(Vector2D newPosition);

    bool IsClicked(Vector2D mousePos);

    std::vector<Component*> GetAttachedComponents();

    void SetRotationWithVector(Vector2D vector, float offset = 0);

    int8 GetPrefabID() const { return _prefabID; }

private:
public:

    GameObjectSettings settings;
    Transform transform;
    uint8 instanceID;

    bool isReplicated;

private:

    int8 _prefabID = -1;
    std::string _name;
    std::vector<Component*> _attachedComponents;
};

template <typename T>
T* GameObject::GetComponent() const
{
    for (auto component : _attachedComponents) 
    {
        if (T* childComp = dynamic_cast<T*>(component)) 
        {
            return childComp;
        }
    }

    return nullptr;
}

#endif //THE_FORGE_GAMEOBJECT_H
