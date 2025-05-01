#pragma once
#include "ShaderUniformData.h"

namespace Engine
{
    class IShaderUniformProvider
    {
    public:
        virtual ~IShaderUniformProvider() = default;
        virtual void CollectUniforms(ShaderUniformData& data) = 0;
    };
}
