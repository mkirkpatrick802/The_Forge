#pragma once
#pragma pack(push, 1)
#include <cstdint>

struct PacketHeader
{
    uint16_t sequenceNumber;

    union
    {
        uint32_t totalSize;
        uint32_t byteOffset;
    };
};

#pragma pack(pop)
