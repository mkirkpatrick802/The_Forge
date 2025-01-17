#include "Object.h"
#include "json.hpp"
#include "JsonKeywords.h"

nlohmann::json Engine::Object::SaveObject()
{
    nlohmann::json data;
    data[JsonKeywords::GAMEOBJECT_NAME] = _name;
    //TODO: save transform data (position & rotation)
    return data;
}
