#include "Launcher.h"
#include "Engine/System.h"

int main()
{
    // The Launcher uses no netcode, so it must not require a running Steam
    // client. This has to be set before anything touches System::GetInstance().
    Engine::REQUIRE_GAMER_SERVICES = false;

    // Engine::System is a singleton -- its constructor/destructor do the setup
    // and teardown that the old explicit Init()/CleanUp() pair used to.
    LauncherSettings settings;
    Launcher::Start<LauncherWindow>(settings, glm::vec2(900, 600));
    settings.CleanUp();
}
