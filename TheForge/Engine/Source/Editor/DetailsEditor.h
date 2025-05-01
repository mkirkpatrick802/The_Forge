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
        static void SetSelectedGameObject(Engine::GameObject* go);
        static void ClearSelectedGameObject();

        static void SetSelectedPrefab(std::unique_ptr<Engine::GameObject> prefab);
        static void ClearSelectedPrefab();
        
    private:

        static Engine::GameObject* _selectedGameObject;
        static std::unique_ptr<Engine::GameObject> _selectedPrefab;
    };
}
