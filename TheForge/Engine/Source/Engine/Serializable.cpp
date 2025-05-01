#include "Serializable.h"

void Engine::Serializable::Serialize(json& j) const
{
    for (const auto& field : fields)
        field.serialize(j, field.ptr);
}

void Engine::Serializable::Deserialize(const json& j)
{
    for (const auto& field : fields)
        field.deserialize(j, field.ptr);
}

json Engine::Serializable::ToJson() const
{
    json j;
    Serialize(j);
    return j;
}

void Engine::Serializable::ToJson(json& j) const
{
    Serialize(j);
}

void Engine::Serializable::FromJson(const json& j)
{
    Deserialize(j);
}
