#include "Engine/System.h"
#include "Launcher/Launcher.h"

int main()
{
	const auto settings = Launcher::Start();

	if(settings.engine == Mode::Edit)
	{
		Engine::System::DisplayMessageBox("Started", "Edit Mode");
	}
	else
	{
		Engine::System::DisplayMessageBox("Started", "Play Mode");
	}
}