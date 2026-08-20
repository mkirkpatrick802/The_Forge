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

        // Centres the view on a world position. Opening a prefab uses this: the prefab
        // sits wherever its own transform says, which is nowhere near where the camera
        // was left in the level, so without it a prefab opens onto empty space.
        void FocusOn(glm::vec2 worldPosition);
        
        glm::mat4 GetProjectionMatrix();
        glm::mat4 GetViewMatrix();

        // A pixel inside the scene image -> a world position.
        //
        // Deliberately inverts the very matrices the renderer draws with, rather than
        // re-deriving the transform: the two cannot then disagree. Note that
        // CameraManager::ConvertScreenToWorld is *not* usable here -- it assumes the
        // whole SDL window and needs a gameplay Camera component, which is null by
        // design in editor mode.
        glm::vec2 ViewportToWorld(glm::vec2 viewportPixels);
        
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
