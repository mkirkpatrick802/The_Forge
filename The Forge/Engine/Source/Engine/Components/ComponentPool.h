#pragma once

class GameObject;
namespace Engine
{
    const int MAX_POOL_SIZE = 50;

    class BasePool
    {
        
    };
    
    template<typename T>
    class ComponentPool final : public BasePool
    {
    public:

        ComponentPool();
        ~ComponentPool() = default;
        
        void Update(float deltaTime);

        T* New(GameObject* go);
        void Delete(T* component);

        T components[MAX_POOL_SIZE];
        bool isActive[MAX_POOL_SIZE];
    };

    template<typename T>
    ComponentPool<T>::ComponentPool()
    {
        for (bool& i : isActive)
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
            if(!isActive[i]) continue;

            if (&components[i] == component)
            {
                components[i].~T();
                isActive[i] = false;
            }
        }
    }
}