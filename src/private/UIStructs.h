//
// Created by mKirkpatrick on 2/13/2024.
//

#ifndef THE_FORGE_UISTRUCTS_H
#define THE_FORGE_UISTRUCTS_H

#include <string>
#include <vector>
#include "GameData.h"

struct EditorSettings {
    bool editMode = true;
    bool editModeChanged = false;
};

struct ComponentSettings {
    uint32 ID;
};

struct SpriteRendererSettings : public ComponentSettings{
    string texture;
    Vector2D size;
    int16 sortingLayer;
};

struct GameObjectSettings {
    std::string* name;
    Vector2D* position;

    std::vector<ComponentSettings> componentSettings;
};

#endif //THE_FORGE_UISTRUCTS_H
