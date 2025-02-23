#pragma once
#include "Component.h"
#include "ComponentUtils.h"
#include "glm/glm.hpp"

namespace Engine
{
    struct CameraBounds
    {
        float left, top, right, bottom;
    };
    
    class Camera final : public Component
    {
    public:
        
        Camera();

        void OnActivation() override;
        
    private:
        CameraBounds _bounds;
        glm::mat4 _projection;
        glm::mat4 _view;
        glm::vec2 _refResolution = glm::vec2(320, 240);

    public:
        glm::vec2 GetReferenceResolution() const { return _refResolution; } 
        glm::mat4 GetProjectionMatrix();
        glm::mat4 GetViewMatrix();
        CameraBounds GetBounds() const { return _bounds; }
    };
    
    REGISTER_COMPONENT(Camera)
}
