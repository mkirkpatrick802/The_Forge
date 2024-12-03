#include "Launcher.h"
#include "Engine/System.h"

int main()
{
    Engine::System::Init();

    // Clear any extra memory before memory dump
    {
        LauncherSettings settings;
        Launcher::Start<LauncherWindow>(settings, Vector2D(900, 600));
        settings.CleanUp();
    }
    
    Engine::System::CleanUp();
}
