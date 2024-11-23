#include "Launcher.h"
#include "Engine/System.h"

int main()
{
    Engine::System::Init();
    LauncherSettings settings;
    Launcher::Start<LauncherWindow>(settings, Vector2D(900, 600));
    Engine::System::CleanUp();
}
