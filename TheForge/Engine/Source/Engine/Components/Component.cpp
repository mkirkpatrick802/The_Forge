#include "Component.h"

#include <cstring>

// Reflection-driven serialization of the members a component marks REPLICATE().
//
// This is the field-level half of keeping a delta small. The component-level half is
// Component::isReplicated, which decides whether a component appears in a delta at
// all; this decides how much of it goes when it does.
//
// Members are copied by offset and size alone -- the walker knows nothing about the
// type beyond where it starts and how long it is. That is safe because the generator
// static_asserts every replicated member is trivially copyable, so a REPLICATE() on a
// std::string is a compile error naming the member rather than a pointer on the wire.
//
// bool is the one special case: OutputByteStream::Write(bool) spends a single bit, and
// copying sizeof(bool) would spend eight. Read and write must agree, so both sides
// branch on the same condition.

namespace
{
    bool IsBool(const MemberInfo& member)
    {
        return member.type == "bool";
    }
}

bool Engine::Component::WriteReplicatedMembers(NetCode::OutputByteStream& stream) const
{
    const ReflectionInfo* info = GetInstanceReflectionInfo();
    if (info == nullptr) return false;

    const auto* base = reinterpret_cast<const uint8_t*>(this);

    bool wroteAny = false;
    for (const MemberInfo& member : info->members)
    {
        if (!member.replicate) continue;

        if (IsBool(member))
            stream.Write(*reinterpret_cast<const bool*>(base + member.offset));
        else
            stream.WriteBytes(base + member.offset, member.size);

        wroteAny = true;
    }

    return wroteAny;
}

bool Engine::Component::ReadReplicatedMembers(NetCode::InputByteStream& stream)
{
    const ReflectionInfo* info = GetInstanceReflectionInfo();
    if (info == nullptr) return false;

    auto* base = reinterpret_cast<uint8_t*>(this);

    bool readAny = false;
    for (const MemberInfo& member : info->members)
    {
        if (!member.replicate) continue;

        if (IsBool(member))
            stream.Read(*reinterpret_cast<bool*>(base + member.offset));
        else
            stream.ReadBytes(base + member.offset, member.size);

        readAny = true;
    }

    return readAny;
}

void Engine::Component::WriteDelta(NetCode::OutputByteStream& stream) const
{
    // Falling back to the full Write is what makes adoption incremental: setting
    // isReplicated starts replicating the whole component immediately, and marking its
    // fields with REPLICATE() narrows it later with no further change here.
    if (!WriteReplicatedMembers(stream))
        Write(stream);
}

void Engine::Component::ReadDelta(NetCode::InputByteStream& stream)
{
    if (!ReadReplicatedMembers(stream))
        Read(stream);
}
