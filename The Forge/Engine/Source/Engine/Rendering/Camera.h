#pragma once
#include "Engine/Component.h"
#include "Engine/Data.h"
#include "glm/glm.hpp"

namespace Engine
{
    class Camera : public Component
    {
    public:
        
        Camera();
        void CleanUp() override;
        
        void LoadData(const json& data) override;
        nlohmann::json SaveData() override;
    
    public:
    
        
    private:
        
        glm::mat4 _projection;
        glm::mat4 _view;
        Vector2D _refResolution = Vector2D(320, 240);
        static const uint32 ComponentID = CAMERA;

    public:
        
        uint32 GetComponentID() const override { return ComponentID; }
        Vector2D GetReferenceResolution() const { return _refResolution; } 
        glm::mat4 GetProjectionMatrix();
        glm::mat4 GetViewMatrix();
    };
}
