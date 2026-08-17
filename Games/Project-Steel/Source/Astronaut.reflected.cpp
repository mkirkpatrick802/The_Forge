#include "Astronaut.h"
#include <cstddef>

ReflectionInfo* Astronaut::GetReflectionInfo() {
    static ReflectionInfo info = {
        "Astronaut", {
            MemberInfo{"_state", offsetof(Astronaut, _state), "EAstronautMoveState", false},
            MemberInfo{"_buildMode", offsetof(Astronaut, _buildMode), "bool", false},
            MemberInfo{"_zoomSpeed", offsetof(Astronaut, _zoomSpeed), "float", false},
            MemberInfo{"_flySpeed", offsetof(Astronaut, _flySpeed), "float", false},
            MemberInfo{"_walkSpeed", offsetof(Astronaut, _walkSpeed), "float", false},
            MemberInfo{"_walkVelocity", offsetof(Astronaut, _walkVelocity), "glm::vec2", false},
            MemberInfo{"_movementInput", offsetof(Astronaut, _movementInput), "glm::vec2", false},
            MemberInfo{"_aimRotation", offsetof(Astronaut, _aimRotation), "float", false},
        }
    };
    return &info;
}
