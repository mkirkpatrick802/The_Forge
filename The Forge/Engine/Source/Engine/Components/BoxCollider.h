#pragma once
#include "ComponentUtils.h"

namespace Engine
{
    class BoxCollider final : public Component
    {
    public:
        BoxCollider();
        
        void Start() override;
        bool CheckCollision(const GameObject* go) const;
        bool CheckCollision(glm::vec2 position) const;

    private:
        glm::vec2 size;
        
    };

    REGISTER_COMPONENT(BoxCollider);
}
