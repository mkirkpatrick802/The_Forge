#include "Destructible.h"
#include "Engine/JsonKeywords.h"
#include "Engine/Level.h"
#include "Engine/System.h"
#include "Engine/Components/SpriteRenderer.h"

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

void Destructible::TakeDamage(Engine::GameObject* dealer, const float damage, int& resourceGain)
{
    _health -= damage;
    if (_health <= 0)
    {
        resourceGain =+ _resourceGain;
        Destroy(gameObject);
    }
}

void Destructible::CollectUniforms(Engine::ShaderUniformData& data)
{
    data.floatUniforms["health"] = GetHealthPercent();

    if (const auto sprite = gameObject->GetComponent<SpriteRenderer>())
        data.vec2Uniforms["size"] = sprite->GetSize();
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
