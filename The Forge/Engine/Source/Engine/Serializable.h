#pragma once
#include <glm/vec2.hpp>

#include "json.hpp"
#include "Components/Component.h"

using json = nlohmann::json;

#define SERIALIZE(type, name) \
    type name; \
    bool _registered_##name = RegisterField(#name, &(name));

namespace Engine
{
    class Serializable
    {
    public:
        virtual ~Serializable() = default;
        
        virtual void Serialize(json& j) const;
        virtual void Deserialize(const json& j);

        json ToJson() const;
        void ToJson(json& j) const;
        
        void FromJson(const json& j);
    protected:
        
        struct FieldInfo
        {
            std::string name;
            void* ptr;
            std::function<void(json&, const void*)> serialize;
            std::function<void(const json&, void*)> deserialize;
        };

        std::vector<FieldInfo> fields;

        template <typename T>
        bool RegisterField(const std::string& name, T* field)
        {
            fields.push_back({
            name, field,
            [&name](json& j, const void* ptr) {j[name] = *static_cast<const T*>(ptr);},
            [&name](const json& j, void* ptr) {if (j.contains(name)) j.at(name).get_to(*static_cast<T*>(ptr));},
            });

            return true;
        }

        template <>
        bool RegisterField<glm::vec2>(const std::string& name, glm::vec2* field)
        {
            fields.push_back({
                .name= name, .ptr= field,
                .serialize= [&name](json j, const void* ptr)
                {
                    const glm::vec2& v = *static_cast<const glm::vec2*>(ptr);
                    j[name] = { v.x, v.y};
                },
                .deserialize= [&name](const json& j, void* ptr)
                {
                    if (j.contains(name))
                    {
                        glm::vec2& v = *static_cast<glm::vec2*>(ptr);
                        v = glm::vec2(j.at(name)[0].get<float>(), j.at(name)[1].get<float>());
                    }
                }
            });

            return true;
        }
    };
}
