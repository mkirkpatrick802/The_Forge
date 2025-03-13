#pragma once
#include "Component.h"
#include <vector>
#include <memory>

class GameObject;
namespace Engine
{
    const int INITIAL_POOL_SIZE = 50;
    const float GROWTH_FACTOR = 1.5f;

    class BasePool
    {
    public:
        BasePool() = default;
        virtual ~BasePool() = default;
        virtual void Update(float deltaTime) {}
    };

    template<typename T>
    class ComponentPool final : public BasePool
    {
        static_assert(std::is_base_of_v<Component, T>, "T must be derived from Component");

    public:
        ComponentPool();
        ~ComponentPool() override = default;

        void Update(float deltaTime) override;

        T* New(GameObject* go);
        void Delete(T* component);

        std::vector<T*> GetActive();

    private:
        void ExpandPool();

        std::vector<std::unique_ptr<T>> components;
        std::vector<bool> isActive;
        std::vector<bool> isStarted;
    };

    template<typename T>
    ComponentPool<T>::ComponentPool() : BasePool()
    {
        components.reserve(INITIAL_POOL_SIZE);
        isActive.reserve(INITIAL_POOL_SIZE);
        isStarted.reserve(INITIAL_POOL_SIZE);

        for (int i = 0; i < INITIAL_POOL_SIZE; i++)
        {
            components.emplace_back(std::make_unique<T>());
            isActive.push_back(false);
            isStarted.push_back(false);
        }
    }

    template<typename T>
    void ComponentPool<T>::Update(float deltaTime)
    {
        for (size_t i = 0; i < components.size(); i++)
        {
            if (isActive[i])
            {
                if (!isStarted[i])
                {
                    components[i]->Start();
                    isStarted[i] = true;
                }

                components[i]->Update(deltaTime);
            }
        }
    }

    template <typename T>
    T* ComponentPool<T>::New(GameObject* go)
    {
        for (size_t i = 0; i < components.size(); i++)
        {
            if (!isActive[i])
            {
                T* next = components[i].get();
                next->gameObject = go;
                next->OnActivation();
                isActive[i] = true;
                return next;
            }
        }

        // No free slot found, expand pool
        ExpandPool();
        return New(go);  // Try again after expanding
    }

    template <typename T>
    void ComponentPool<T>::ExpandPool()
    {
        size_t oldSize = components.size();
        size_t newSize = static_cast<size_t>(oldSize * GROWTH_FACTOR) + 1;

        components.reserve(newSize);
        isActive.reserve(newSize);
        isStarted.reserve(newSize);

        for (size_t i = oldSize; i < newSize; i++)
        {
            components.emplace_back(std::make_unique<T>());
            isActive.push_back(false);
            isStarted.push_back(false);
        }
    }

    template <typename T>
    void ComponentPool<T>::Delete(T* component)
    {
        for (size_t i = 0; i < components.size(); i++)
        {
            if (!isActive[i]) continue;
            if (components[i].get() == component)
            {
                components[i].reset();
                components[i] = std::make_unique<T>();
                
                isActive[i] = false;
                isStarted[i] = false;
            }
        }
    }

    template <typename T>
    std::vector<T*> ComponentPool<T>::GetActive()
    {
        std::vector<T*> active;
        for (size_t i = 0; i < components.size(); i++)
        {
            if (!isActive[i]) continue;
            active.push_back(components[i].get());
        }

        return active;
    }
}