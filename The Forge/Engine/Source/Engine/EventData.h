#pragma once
#include "json.hpp"
#include "GameObject.h"
#include "SpriteRenderer.h"

namespace Engine
{
    #define DECLARE_EVENT_NAME(eventType) \
    static std::string GetEventName() { return #eventType; }
    
    struct ED_CreateSpriteRenderer
    {
        DECLARE_EVENT_NAME(ED_CreateSpriteRenderer)
        
        GameObject* gameObject;
        nlohmann::json data;
    };

    struct ED_DestroySpriteRenderer
    {
        DECLARE_EVENT_NAME(ED_DestroySpriteRenderer)
        
        SpriteRenderer* spriteRenderer;
    };
}
