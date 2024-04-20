#pragma once

#include <thread>

#include "EditorUIWindow.h"
#include "Engine.h"
#include "EventListener.h"
#include "UIStructs.h"

class UIManager;

class EditorEngine : public Engine, public EventListener
{
public:

    EditorEngine(Renderer &renderer, InputManager &inputManager);

    virtual void GameLoop() override;
    virtual void CleanUp() override;

    void ClickObject();

    virtual void OnEvent(Event* event) override;

private:

    Details _details;
    EditorUIWindow _editorUI;
};