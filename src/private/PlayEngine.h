#pragma once
#include "Engine.h"

class NetCode;
class PlayEngine : public Engine
{

public:

    PlayEngine(Renderer &renderer, InputManager &inputManager, NetCode* netcode);

    void GameLoop() override;

    void CleanUp() override;

private:

    NetCode* _netcode;

};