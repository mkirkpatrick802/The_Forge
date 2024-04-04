#pragma once

#include "Renderer.h"
#include "UIStructs.h"
#include "EventDispatcher.h"

class UIManager : public EventDispatcher
{
public:

    UIManager(Renderer& renderer);

    bool HoveringUI();

    void Render(EditorSettings &editorSettings, const GameObjectSettings* selectedGameObjectSettings);
    void CleanUp();

    static void ClearFrame();

private:

    Renderer _renderer;

    // Conditional Logic
    bool _playerSpawned = false;

};