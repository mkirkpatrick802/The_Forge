#pragma once
#include <string>
#include <vector>

struct MemberInfo {
    std::string name;
    size_t offset;
    std::string type;
    bool replicate;
};

struct ReflectionInfo {
    std::string className;
    std::vector<MemberInfo> members;

    const MemberInfo* FindMember(const std::string& name) const;
};