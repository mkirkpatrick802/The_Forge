#pragma once
#include <string>
#include <unordered_map>
#include <map>
#include <glm/glm.hpp>

namespace Engine
{
    struct ShaderUniformData
    {
        std::unordered_map<std::string, float> floatUniforms;
        std::unordered_map<std::string, int> intUniforms;
        std::map<std::string, glm::vec2> vec2Uniforms;
        std::map<std::string, glm::vec3> vec3Uniforms;
        std::map<std::string, glm::vec4> vec4Uniforms;
        std::map<std::string, glm::mat4> mat4Uniforms;
    };
}
