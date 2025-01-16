#pragma once
#include "Engine/Data.h"
#include "glm/glm.hpp"

namespace Engine
{
    class Camera
    {
    public:

        Camera();

        glm::mat4 GetProjectionMatrix();
        
        Vector2D ConvertWorldToScreen(Vector2D worldPos);
        
    private:
        
        glm::mat4 _projection;
    };
}
