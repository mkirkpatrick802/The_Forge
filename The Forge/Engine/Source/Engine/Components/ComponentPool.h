#pragma once
#include "Component.h"

class GameObject;
namespace Engine
{
    const int MAX_POOL_SIZE = 50;

    class BasePool
    {
    public:
        virtual void Update(float deltaTime) {}
    };
    
    template<typename T>
    class ComponentPool final : public BasePool
    {
        static_assert(std::is_base_of_v<Component, T>, "T must be derived from Component");
        
    public:
        ComponentPool();
        ~ComponentPool() = default;
        
        void Update(float deltaTime) override;

        T* New(GameObject* go);
        void Delete(T* component);

        std::vector<T*> GetActive();

    private:
        T components[MAX_POOL_SIZE];
        bool isActive[MAX_POOL_SIZE];
        bool isStarted[MAX_POOL_SIZE];
    };

    template<typename T>
    ComponentPool<T>::ComponentPool()
    {
        for (bool& i : isActive)
        {
            i = false;
        }

        for (bool& i : isStarted)
        {
            i = false;
        }
    }

    template<typename T>
    void ComponentPool<T>::Update(float deltaTime)
    {
        for (int i = 0; i < MAX_POOL_SIZE; i++)
        {
            if (isActive[i])
            {
                if (!isStarted[i])
                {
                    components[i].Start();
                    isStarted[i] = true;
                }
                
                components[i].Update(deltaTime);
            }
        }
    }

    template <typename T>
    T* ComponentPool<T>::New(GameObject* go)
    {
        for (int i = 0; i < MAX_POOL_SIZE; i++)
        {
            if (!isActive[i])
            {
                T* next = &components[i];
                next->gameObject = go;
                next->OnActivation();
                isActive[i] = true;
                return next;
            }
        }
        
        return nullptr;
    }

    template <typename T>
    void ComponentPool<T>::Delete(T* component)
    {
        for (int i = 0; i < MAX_POOL_SIZE; i++)
        {
            if (!isActive[i]) continue;

            if (&components[i] == component)
            {
                components[i].~T();
                new (&components[i]) T();
                
                isActive[i] = false;
                isStarted[i] = false;
            }
        }
    }

    template <typename T>
    std::vector<T*> ComponentPool<T>::GetActive()
    {
        std::vector<T*> active;
        for (int i = 0; i < MAX_POOL_SIZE; i++)
        {
            if (!isActive[i]) continue;
            active.push_back(&components[i]);
        }

        return active;
    }
}
