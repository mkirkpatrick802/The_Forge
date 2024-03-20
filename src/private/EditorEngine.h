//
// Created by mKirkpatrick on 2/27/2024.
//

#ifndef THE_FORGE_EDITORENGINE_H
#define THE_FORGE_EDITORENGINE_H

#include "Engine.h"
#include "UIStructs.h"

class UIManager;

class EditorEngine : public Engine {

public:

    EditorEngine(Renderer &renderer, InputManager &inputManager, UIManager &uiManager);

    void GameLoop();

    void ClickObject();

    void CleanUp() override;

private:

    EditorSettings _editorSettings;
    GameObjectSettings* _selectedGameObjectSettings;

    UIManager* _uiManager;
};


#endif //THE_FORGE_EDITORENGINE_H
