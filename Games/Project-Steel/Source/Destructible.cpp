#include "Destructible.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Level.h"
#include "Engine/System.h"

using namespace Engine;

Destructible::Destructible(): _health(0)
{
    
}

void Destructible::OnActivation()
{
    _health = _maxHealth;
}

void Destructible::Start()
{
    
}

void Destructible::TakeDamage(Engine::GameObject* dealer, const float damage)
{
    _health -= damage;
    if (_health <= 0)
        Destroy(gameObject);
}

void Destructible::Deserialize(const json& data)
{
    
}

nlohmann::json Destructible::Serialize()
{
    nlohmann::json data;
    data[JsonKeywords::COMPONENT_ID] = GetComponentRegistry().GetComponentID<Destructible>();
    return data;
}

// TODO: Need to figure out how to write ImGui stuff
void Destructible::DrawDetails()
{
    
}
