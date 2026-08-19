#include "Rigidbody.h"
#include <cstddef>
#include <type_traits>

namespace Engine {

ReflectionInfo* Rigidbody::GetReflectionInfo() {
    static_assert(std::is_trivially_copyable_v<decltype(Rigidbody::_velocity)>,
        "REPLICATE() on Rigidbody::_velocity: only trivially copyable members can be replicated by reflection.");
    static_assert(std::is_trivially_copyable_v<decltype(Rigidbody::_acceleration)>,
        "REPLICATE() on Rigidbody::_acceleration: only trivially copyable members can be replicated by reflection.");

    static ReflectionInfo info = {
        "Engine::Rigidbody", {
            MemberInfo{"_velocity", offsetof(Rigidbody, _velocity), sizeof(Rigidbody::_velocity), "glm::vec2", true},
            MemberInfo{"_acceleration", offsetof(Rigidbody, _acceleration), sizeof(Rigidbody::_acceleration), "glm::vec2", true},
            MemberInfo{"_useAreaAsMass", offsetof(Rigidbody, _useAreaAsMass), sizeof(Rigidbody::_useAreaAsMass), "bool", false},
            MemberInfo{"_mass", offsetof(Rigidbody, _mass), sizeof(Rigidbody::_mass), "float", false},
            MemberInfo{"_density", offsetof(Rigidbody, _density), sizeof(Rigidbody::_density), "float", false},
            MemberInfo{"_inverseMass", offsetof(Rigidbody, _inverseMass), sizeof(Rigidbody::_inverseMass), "float", false},
            MemberInfo{"_static", offsetof(Rigidbody, _static), sizeof(Rigidbody::_static), "bool", false},
            MemberInfo{"_friction", offsetof(Rigidbody, _friction), sizeof(Rigidbody::_friction), "float", false},
        }
    };
    return &info;
}

} // namespace Engine
