#include "ReflectionUtils.h"

const MemberInfo* ReflectionInfo::FindMember(const std::string& name) const {
    for (const auto& member : members) {
        if (member.name == name) {
            return &member;
        }
    }
    return nullptr;
}