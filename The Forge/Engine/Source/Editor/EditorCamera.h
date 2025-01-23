#pragma once
#include <memory>

#include "Engine/Data.h"
#include "glm/glm.hpp"

namespace Editor
{
    class EditorCamera
    {
    public:

        static std::shared_ptr<EditorCamera> GetInstance();

        EditorCamera() = default;
        ~EditorCamera() = default;

        void Update(float deltaTime);
        
        glm::mat4 GetProjectionMatrix();
        glm::mat4 GetViewMatrix();

    private:

        static std::shared_ptr<EditorCamera> _instance;

        Vector2D _position;
        
        glm::mat4 _projection;
        glm::mat4 _view;
    };
}
