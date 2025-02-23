#pragma once
#include "Component.h"
#include "ComponentUtils.h"

namespace Engine
{
    class PlayerStart final : public Component
    {
    public:
        PlayerStart() = default;
        
    };

    REGISTER_COMPONENT(PlayerStart);
}
