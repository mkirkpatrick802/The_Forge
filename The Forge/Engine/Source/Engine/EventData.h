#pragma once
#include "json.hpp"
#include "System.h"

namespace Engine
{
    #define DECLARE_EVENT_NAME(eventType) \
    static std::string GetEventName() { return #eventType; }

    struct ED_LogToConsole
    {
        DECLARE_EVENT_NAME(ED_LogToConsole)

        std::string message;
        LogType type;
    };
}
