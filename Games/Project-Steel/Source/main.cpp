#include "Editor/LevelEditor.h"
#include "Engine/GameEngine.h"
#include "Engine/System.h"
#include "Engine/UIManager.h"

using namespace Engine;
using namespace Editor;

int main()
{
    System::Init();

    {
        System::CreateAppWindow(Vector2D(800, 600));
        auto instance = GameEngine::GetInstance();

        const auto window = DEBUG_NEW LevelEditor();
        UIManager::AddUIWindow(window);
        instance->StartGamePlayLoop();
        delete window;
        
        instance->CleanUp();
        delete instance;
        instance = nullptr;
    }
    
    System::CleanUp();
}
