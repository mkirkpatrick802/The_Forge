#pragma once
#include "Engine/UIWindow.h"

namespace Engine
{
    class GameObject;
}

namespace Editor
{
    class DetailsEditor final : public Engine::UIWindow
    {
    public:
        void Render() override;

    private:
        
    public:

        static void SetSelectedGameObject(Engine::GameObject* go);
        static void ClearSelectedGameObject() { _selectedGameObject = nullptr; }
        
    private:

        static Engine::GameObject* _selectedGameObject;
    };
}
