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
        
        Vector2D ConvertWorldToScreen(Vector2D worldPos) const;
    

    public:
    
        
    private:
        
        glm::mat4 _projection;
        static const uint32 ComponentID = CAMERA;

    public:
        
        uint32 GetComponentID() const override { return ComponentID; }
        glm::mat4 GetProjectionMatrix();
        
    };
}
