#include "Astronaut.h"

ReflectionInfo* Astronaut::GetReflectionInfo() {
    static ReflectionInfo info = {
        "Astronaut", {
            MemberInfo{"EAstronautMoveState", offsetof(Astronaut, _state), "_state", false},
            MemberInfo{"bool", offsetof(Astronaut, _buildMode), "_buildMode", false},
            MemberInfo{"float", offsetof(Astronaut, _zoomSpeed), "_zoomSpeed", false},
            MemberInfo{"float", offsetof(Astronaut, _flySpeed), "_flySpeed", false},
            MemberInfo{"float", offsetof(Astronaut, _walkSpeed), "_walkSpeed", false},
        }
    };
    return &info;
}
