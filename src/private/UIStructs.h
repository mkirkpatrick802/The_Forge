#pragma once

#include <string>
#include <vector>
#include "GameData.h"

struct EditorSettings
{
    bool editMode = true;
};

struct ComponentSettings
{
    uint32 ID;
};

struct SpriteRendererSettings : public ComponentSettings
{
    string texture;
    Vector2D size;
    int16 sortingLayer;
};

struct GameObjectSettings
{
    std::string* name;
    bool* isReplicated;
    Vector2D* position;

    std::vector<ComponentSettings> componentSettings;
};

struct Details
{
    EditorSettings editorSettings;
    GameObjectSettings* gameObjectSettings;

    Details& operator=(const Details& other)
	{
        if (this != &other) 
        {
            editorSettings = other.editorSettings;

            delete gameObjectSettings;
            if (other.gameObjectSettings)
            {
                gameObjectSettings = new GameObjectSettings(*other.gameObjectSettings);
            }
        }
        return *this;
    }

};