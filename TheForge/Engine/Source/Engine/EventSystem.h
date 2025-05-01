#pragma once
#include <functional>

#include "System.h"

namespace Engine
{
    // Custom hash for std::pair<const void*, std::string>
    struct pair_hash {
        template <typename T1, typename T2>
        std::size_t operator ()(const std::pair<T1, T2>& p) const {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            return h1 ^ (h2 << 1);  // Combine hashes
        }
    };
    
    using EventCallback = std::function<void(const void*)>;
    class EventSystem
    {
    public:
        
        static std::shared_ptr<EventSystem> GetInstance();
        static void DestroyInstance();
        EventSystem() = default;
        ~EventSystem() = default;
        
        template <typename T>
        void RegisterEvent(const std::string& name, T* obj, void (T::*func)(const void*));
        void RegisterEvent(const std::string& name, void* obj, const EventCallback& callback);
        
        void TriggerEvent(const std::string& name, const void* data = nullptr);
        void DeregisterEvent(const std::string& name, void* obj);
        
    private:
        

        EventSystem(const EventSystem&) = delete;
        EventSystem(EventSystem&&) = delete;
        EventSystem& operator=(const EventSystem&) = delete;
        EventSystem& operator=(EventSystem&&) = delete;

        static std::shared_ptr<EventSystem> _instance;
        
        // Map each event name to a map of objects and their associated callbacks
        std::unordered_map<std::string, std::unordered_map<void*, std::vector<EventCallback>>> _eventMap;
    };

    template <typename T>
    void EventSystem::RegisterEvent(const std::string& name, T* obj, void(T::* func)(const void*))
    {
        //const auto pair = std::make_pair(static_cast<const void*>(obj), name);
        _eventMap[name][static_cast<void*>(obj)].push_back([=](const void* data) {(obj->*func)(data);});
    }
}
