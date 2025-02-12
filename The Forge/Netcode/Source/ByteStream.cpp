#include "ByteStream.h"

#include "LinkingContext.h"
#include "Engine/GameObject.h"
#include "Engine/GameObject.h"
#include "Engine/Components/ComponentFactories.h"

/*
 *      Output Byte Stream
 */
NetCode::OutputByteStream::OutputByteStream() : _buffer(nullptr), _head(0)
{
    ReallocBuffer(1500 * 8);
}

void NetCode::OutputByteStream::WriteBits(const uint8_t data, const size_t size)
{
    auto& buffer = *_buffer;
    const auto nextHead = _head + static_cast<uint32_t>(size);
    if (nextHead > _capacity)
        ReallocBuffer(std::max(_capacity * 2, nextHead));

    const uint32_t byteOffset = _head >> 3;
    const uint32_t bitOffset = _head & 0x7;

    const auto currentMask = static_cast<uint8_t>(~(0xff << bitOffset));
    buffer[byteOffset] = static_cast<uint8_t>((buffer[byteOffset] & currentMask) | (data << bitOffset));

    if (const uint32_t bitsFreeThisByte = 8 - bitOffset; bitsFreeThisByte < size)
        buffer[byteOffset + 1] = data >> bitsFreeThisByte;

    _head = nextHead;
}

void NetCode::OutputByteStream::WriteBits(const void* data, size_t size)
{
    auto srcByte = static_cast<const char*>(data);
    while (size > 8)
    {
        WriteBits(*srcByte, 8);
        ++srcByte;
        size -= 8;
    }

    if (size > 0)
        WriteBits(*srcByte, size);
}

void NetCode::OutputByteStream::Write(const glm::vec2& data)
{
    Write(data.x);
    Write(data.y);
}

void NetCode::OutputByteStream::Write(const std::string& data)
{
    const auto elementCount = static_cast<uint32_t>(data.size());
    Write(elementCount);
    
    for (const auto& element : data)
        Write(element);
}

void NetCode::OutputByteStream::ReallocBuffer(const uint32_t newSize)
{
    if (_buffer == nullptr)
    {
        _buffer = std::make_shared<std::vector<uint8_t>>(newSize >> 3, 0);
    }
    else
    {
        _buffer->resize(newSize >> 3, 0);
    }

    _capacity = newSize;
}

/*
 *      Input Byte Stream
 */

NetCode::InputByteStream::InputByteStream(uint32_t size): _head(0), _capacity(0)
{
    _buffer = std::make_shared<std::vector<uint8_t>>(size * 8, 0);
}

NetCode::InputByteStream::InputByteStream(const InputByteStream& other) : _head(other._head), _capacity(other._capacity)
{
    auto byteCount = static_cast<int>(_capacity / 8);
    _buffer = std::make_shared<std::vector<uint8_t>>(byteCount);
    std::memcpy(_buffer->data(), other._buffer->data(), byteCount);
}

void NetCode::InputByteStream::ReadBits(uint8_t& data, uint32_t size)
{
    const auto& buffer = *_buffer;
    const uint32_t byteOffset = _head >> 3;
    const uint32_t bitOffset = _head & 0x7;

    data = buffer[byteOffset] >> bitOffset;

    if (const uint32_t bitsFreeThisByte = 8 - bitOffset; bitsFreeThisByte < size)
        data |= buffer[byteOffset + 1] << bitsFreeThisByte;

    data &= ~(0x00ff << size);
    _head += size;
}

void NetCode::InputByteStream::ReadBits(void* data, uint32_t size)
{
    auto destByte = static_cast<uint8_t*>(data);
    while (size > 8)
    {
        ReadBits(*destByte, 8);
        ++destByte;
        size -= 8;
    }

    if (size > 0)
        ReadBits(*destByte, size);
}

void NetCode::InputByteStream::Read(glm::vec2& data)
{
    Read(data.x);
    Read(data.y);
}

void NetCode::InputByteStream::Read(std::string& data)
{
    uint32_t elementCount;
    Read(elementCount);
    data.resize(elementCount);
    
    for (auto& element : data)
        Read(element);
}