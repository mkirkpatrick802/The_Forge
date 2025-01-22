#pragma once
#include "json.hpp"
#include "GameObject.h"
#include "SpriteRenderer.h"
#include "System.h"

namespace Engine
{
    #define DECLARE_EVENT_NAME(eventType) \
    static std::string GetEventName() { return #eventType; }
    
    struct ED_CreateComponent
    {
        DECLARE_EVENT_NAME(ED_CreateComponent)
        
        GameObject* gameObject;
        nlohmann::json data;
        uint32 componentID;
    };

    struct ED_DestroyComponent
    {
        DECLARE_EVENT_NAME(ED_DestroyComponent)
        
        Component* component;
    };

    struct ED_LogToConsole
    {
        DECLARE_EVENT_NAME(ED_LogToConsole)

        String message;
        LogType type;
    };
}
