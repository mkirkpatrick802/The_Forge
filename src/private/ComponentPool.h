//
// Created by mKirkpatrick on 1/30/2024.
//

#ifndef THE_FORGE_COMPONENTPOOL_H
#define THE_FORGE_COMPONENTPOOL_H

#include <cassert>
#include "GameData.h"

class GameObject;

template<typename T>
class ComponentPool {

public:

    ComponentPool();
    void Update(float deltaTime);
    //void Render(SDL_Renderer* renderer);

    T* New(GameObject* go);
    T* Delete(T* component);

public:

    T components[MAX_GAMEOBJECTS];
    bool isActive[MAX_GAMEOBJECTS];
};

template<typename T>
ComponentPool<T>::ComponentPool()
{
    for (int i = 0; i < MAX_GAMEOBJECTS; i++)
    {
        isActive[i] = false;
    }
}

template<typename T>
void ComponentPool<T>::Update(float deltaTime)
{
    for (int i = 0; i < MAX_GAMEOBJECTS; i++)
    {
        if (isActive[i])
        {
            components[i].Update(deltaTime);
        }
    }
}

/*template <typename T>
void ComponentPool<T>::Render(SDL_Renderer* renderer)
{
    for (int i = 0; i < MAX_GAMEOBJECTS; i++)
    {
        if (isActive[i])
        {
            components[i].Render(renderer);
        }
    }
}*/

template <typename T>
T* ComponentPool<T>::New(GameObject* go)
{
    for (int i = 0; i < MAX_GAMEOBJECTS; i++)
    {
        if (!isActive[i])
        {
            T* next = &components[i];
            next->gameObject = go;
            isActive[i] = true;
            return next;
        }
    }

    assert(false);
    return nullptr;
}

template <typename T>
T* ComponentPool<T>::Delete(T* component)
{
    for (int i = 0; i < MAX_GAMEOBJECTS; i++)
    {
        if(!isActive[i]) continue;

        if (&components[i] == component)
        {
            isActive[i] = false;
        }
    }

    return nullptr;
}

#endif //THE_FORGE_COMPONENTPOOL_H
