#pragma once
#include "Engine/Components/ComponentUtils.h"
#include "Engine/Rendering/IShaderUniformProvider.h"

class Destructible : public Engine::Component, public Engine::IShaderUniformProvider
{
public:
    Destructible();
    void OnActivation() override;
    void Start() override;

    void TakeDamage(Engine::GameObject* dealer, float damage, int& resourceGain);
    void CollectUniforms(Engine::ShaderUniformData& data) override;

    void Write(NetCode::OutputByteStream& stream) const override;
    void Read(NetCode::InputByteStream& stream) override;
    
    void DrawDetails() override;
    
private:
    float _health;
    float _maxHealth = 100;
    float _resourceGain = 20;

public:
    inline float GetHealthPercent() const { return _health / _maxHealth; }
    
};

REGISTER_COMPONENT(Destructible)