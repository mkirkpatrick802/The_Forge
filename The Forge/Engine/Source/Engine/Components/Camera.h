#pragma once
#include "Component.h"
#include "ComponentRegistry.h"
#include "glm/glm.hpp"

namespace Engine
{
    class Camera : public Component
    {
    public:
        
        Camera();
        
        void LoadData(const json& data) override;
        nlohmann::json SaveData() override;
    
    public:
    
        
    private:
        
        glm::mat4 _projection;
        glm::mat4 _view;
        glm::vec2 _refResolution = glm::vec2(320, 240);

    public:
        
        glm::vec2 GetReferenceResolution() const { return _refResolution; } 
        glm::mat4 GetProjectionMatrix();
        glm::mat4 GetViewMatrix();
    };

    REGISTER_COMPONENT(Camera)
    
}
