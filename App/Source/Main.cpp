#include "Engine/System.h"

int main()
{
	Engine::System system;
	system.Init();
	system.PreAppStartUp();

	const auto i = new int;
	*i = 6;

	system.PostAppStartUp();
	system.Shutdown();
}