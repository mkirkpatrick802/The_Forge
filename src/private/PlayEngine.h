//
// Created by mKirkpatrick on 2/27/2024.
//

#ifndef THE_FORGE_PLAYENGINE_H
#define THE_FORGE_PLAYENGINE_H


#include "Engine.h"

class NetCode;

class PlayEngine : public Engine {

public:

    PlayEngine(Renderer &renderer, InputManager &inputManager, NetCode* netcode);

    void GameLoop() override;

    void CleanUp() override;

private:

    NetCode* _netcode;

};


#endif //THE_FORGE_PLAYENGINE_H
