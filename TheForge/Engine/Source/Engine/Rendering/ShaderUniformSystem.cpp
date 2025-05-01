#include "ShaderUniformSystem.h"

#include "IShaderUniformProvider.h"
#include "Engine/GameObject.h"

Engine::ShaderUniformData Engine::ShaderUniformSystem::CollectUniforms(const GameObject* go)
{
    ShaderUniformData data;
    for (auto* component : go->GetAllComponents())
        if (const auto provider = dynamic_cast<IShaderUniformProvider*>(component))
            provider->CollectUniforms(data);

    return data;
}