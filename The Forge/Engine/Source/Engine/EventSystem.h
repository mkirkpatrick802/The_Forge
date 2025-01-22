#pragma once
#include <functional>

#include "Data.h"
#include "System.h"

namespace Engine
{
    using EventCallback = std::function<void(const void*)>;
    class EventSystem
    {
    public:
        
        static std::shared_ptr<EventSystem> GetInstance();
        static void DestroyInstance();
        EventSystem() = default;
        ~EventSystem() = default;
        
        template <typename T>
        void RegisterEvent(const String& name, T* obj, void (T::*func)(const void*));
        void RegisterEvent(const String& name, const EventCallback& callback);
        
        void TriggerEvent(const String& name, const void* data = nullptr);
        void DeregisterEvent(const String& name);
        
    private:
        

        EventSystem(const EventSystem&) = delete;
        EventSystem(EventSystem&&) = delete;
        EventSystem& operator=(const EventSystem&) = delete;
        EventSystem& operator=(EventSystem&&) = delete;

        static std::shared_ptr<EventSystem> _instance;
        std::unordered_map<std::string, std::vector<EventCallback>> _eventMap;
    };

    template <typename T>
    void EventSystem::RegisterEvent(const String& name, T* obj, void(T::* func)(const void*))
    {
        _eventMap[name].push_back([=](const void* data) {(obj->*func)(data);});
    }
}
