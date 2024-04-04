#pragma once

#include "EditorUIWindow.h"
#include "Engine.h"
#include "UIStructs.h"

class UIManager;

class EditorEngine : public Engine, public EventListener
{
public:

    EditorEngine(Renderer &renderer, InputManager &inputManager);

    virtual void GameLoop() override;
    virtual void CleanUp() override;

    void ClickObject();

    virtual void OnEvent(EventType event) override;

private:

    Details _details;
    EditorUIWindow _editorUI;
};