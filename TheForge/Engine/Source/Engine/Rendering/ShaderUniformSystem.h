#pragma once
#include "ShaderUniformData.h"

namespace Engine
{
    class GameObject;
    class ShaderUniformSystem
    {
    public:
        static ShaderUniformData CollectUniforms(const GameObject* go);
    };
}
