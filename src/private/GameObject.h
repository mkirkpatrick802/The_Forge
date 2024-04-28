#pragma once

#include <vector>
#include <string>

#include "EventDispatcher.h"
#include "Transform.h"
#include "UIStructs.h"

class Component;

class GameObject : public EventDispatcher
{
public:

    friend class ObjectCreator;
    friend class GameObjectManager;
    friend class ObjectStateReader;

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

    void SetShouldRespawn(const bool shouldRespawn) { _shouldRespawn = shouldRespawn; }

    // Can be called anywhere and automatically replicates to clients
    void Destroy();

private:

    // Should only ever be called from ObjectStateReader class
    void DestroyRequestReceived();

public:

    GameObjectSettings settings;
    Transform transform;
    uint8 instanceID;

    bool isReplicated;
    GameObject* owner = nullptr;
    std::string name;

private:

    bool _shouldRespawn = false;
    int8 _prefabID = -1;
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