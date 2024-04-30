#include "Engine/GameEngine.h"

using namespace Engine;

int main()
{
	GameEngine* engine = GameEngine::GetInstance();

	engine->StartGamePlayLoop();
	engine->CleanUp();
}