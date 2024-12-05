#include "GameObject.h"

#include "JsonKeywords.h"

Engine::GameObject::GameObject()
{
    _data = {
        {JsonKeywords::GAMEOBJECT_NAME, "New Game Object"}
    };

    _name = "New Game Object";
    _attachedComponents.push_back(nullptr);
}

Engine::GameObject::GameObject(const nlohmann::json& data)
{
    
}
