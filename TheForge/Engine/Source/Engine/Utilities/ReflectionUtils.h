#pragma once
#include <string>
#include <vector>

struct MemberInfo {
    std::string name;
    size_t offset;

    // sizeof the member. The generator emits sizeof(Class::member), which is the only
    // place the real type is in scope -- it lets a generic walker copy a member it
    // knows nothing about beyond where it starts and how long it is.
    size_t size;

    std::string type;

    // Marked REPLICATE(). Consumed by Component::WriteDelta to serialize only the
    // members that actually change per tick. The generator static_asserts that a
    // replicated member is trivially copyable, so this is only ever true for a type
    // the generic walker can safely handle.
    bool replicate;
};

struct ReflectionInfo {
    std::string className;
    std::vector<MemberInfo> members;

    const MemberInfo* FindMember(const std::string& name) const;
};