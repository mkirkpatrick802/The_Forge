#pragma once
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

    private:

        static void CleanUpPrefab();
        
    public:

        static void SetSelectedGameObject(Engine::GameObject* go);
        static void ClearSelectedGameObject();
        
    private:

        static Engine::GameObject* _selectedGameObject;
    };
}
