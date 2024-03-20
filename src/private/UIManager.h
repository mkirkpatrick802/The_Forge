//
// Created by mKirkpatrick on 1/22/2024.
//

#ifndef THE_FORGE_UIMANAGER_H
#define THE_FORGE_UIMANAGER_H

#include "Renderer.h"
#include "UIStructs.h"

class UIManager {

public:

    UIManager(Renderer& renderer);

    bool HoveringUI();

    void Render(EditorSettings &editorSettings, GameObjectSettings* selectedGameObjectSettings);
    void CleanUp();

    static void ClearFrame();

protected:
private:

    Renderer _renderer;

};


#endif //THE_FORGE_UIMANAGER_H
