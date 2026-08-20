#pragma once
#include <memory>

#include "Engine/Rendering/UIWindow.h"

namespace Engine
{
    class GameObject;
}

namespace Editor
{
    class DetailsEditor final : public Engine::UIWindow
    {
    public:
        ~DetailsEditor();
        void Render() override;
        
    public:
        static Engine::GameObject* GetSelectedGameObject() { return _selectedGameObject; }
        static void SetSelectedGameObject(Engine::GameObject* go);
        static void ClearSelectedGameObject();
        
    private:

        // Never owns what it points at. A prefab being edited used to be owned here,
        // which meant selecting anything else destroyed it mid-frame -- it is a level
        // object now, and the level owns it like any other. See LevelManager::OpenPrefab.
        static Engine::GameObject* _selectedGameObject;
    };
}
