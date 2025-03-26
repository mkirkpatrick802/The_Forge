#pragma once
#include <memory>
#include "glm/glm.hpp"

namespace Editor
{
    class EditorCamera
    {
    public:

        static std::shared_ptr<EditorCamera> GetInstance();

        EditorCamera();
        ~EditorCamera() = default;

        static void CleanUp();

        void Update();
        void UpdatePosition(glm::vec2 delta);
        
        glm::mat4 GetProjectionMatrix();
        glm::mat4 GetViewMatrix();
        
        void SetZoom(float zoom) { _zoom = glm::clamp(zoom, 0.2f, 10.0f); }  // Clamping for stability
        void ZoomIn(float amount) { SetZoom(_zoom - amount); }
        void ZoomOut(float amount) { SetZoom(_zoom + amount); }

    private:
        static std::shared_ptr<EditorCamera> _instance;

        glm::vec2 _position;
        float _dragSpeed = 1;

        float _zoomSpeed = .3f;
        float _zoom = 1;
        glm::mat4 _projection;
        glm::mat4 _view;
    };
}
