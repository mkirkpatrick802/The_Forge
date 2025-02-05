#include "GameModeBase.h"
#include "Components/ComponentUtils.h"
#include "Components/PlayerStart.h"

Engine::GameModeBase::GameModeBase()
{
    _playerStarts = GetComponentManager().GetPool<PlayerStart>()->GetActive();
}

void Engine::GameModeBase::Start()
{
    
}
