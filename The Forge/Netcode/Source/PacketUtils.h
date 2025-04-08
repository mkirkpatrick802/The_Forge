#pragma once
#include <cstdint>
#include <cstring>
#include "PacketHeader.h"

inline uint32_t BuildPacket(uint8_t* outBuffer, const uint16_t sequence, const uint8_t* chunkData, const uint32_t chunkSize, const uint32_t byteOffset, const uint32_t totalSize)
{
    PacketHeader header;
    header.sequenceNumber = sequence;
    if (sequence == 0)
    {
        header.totalSize = totalSize;
    }
    else
    {
        header.byteOffset = byteOffset;
    }

    std::memcpy(outBuffer, &header, sizeof(PacketHeader));
    std::memcpy(outBuffer + sizeof(PacketHeader), chunkData, chunkSize);

    return sizeof(PacketHeader) + chunkSize;
}
