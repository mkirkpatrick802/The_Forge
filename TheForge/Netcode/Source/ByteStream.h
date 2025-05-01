#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <glm/vec2.hpp>

namespace Engine
{
    class Component;
    class GameObject;
}

namespace NetCode
{
    class OutputByteStream
    {
    public:
        OutputByteStream();
        ~OutputByteStream() = default;

        void WriteBits (uint8_t data, size_t size);
        void WriteBits (const void* data, size_t size);
        void WriteBytes(const void* data, size_t size) { WriteBits(data, size << 3); }

        template<typename T>
        void Write (T data, uint32_t size = sizeof(T) * 8)
        {
            static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>, "Generic Write only supports primitive data types");
            WriteBits(&data, size);
        }

        void Write (const bool data) { WriteBits(&data, 1); }
        void Write (const glm::vec2& data);
        void Write (const std::string& data);
        
        const uint8_t* GetBuffer() const { return _buffer->data(); }
        uint32_t GetBitLength() const { return _head; }
        uint32_t GetByteLength() const { return (_head + 7) >> 3; }
            
    private:
        void ReallocBuffer(uint32_t newSize);

    private:
        std::shared_ptr<std::vector<uint8_t>> _buffer;
        uint32_t _head;
        uint32_t _capacity;
    };

    class InputByteStream
    {
    public:
        InputByteStream(uint32_t size);
        InputByteStream(const InputByteStream& other);
        InputByteStream(const uint8_t* data, uint32_t size);
        ~InputByteStream() = default;

        void ReadBits(uint8_t& data, uint32_t size);
        void ReadBits(void* data, uint32_t size);
        void ReadBytes(void* data, const size_t size) { ReadBits(data, size << 3); }

        template<typename T>
        void Read(T& data, const uint32_t size = sizeof(T) * 8)
        {
            static_assert(std::is_arithmetic_v<T> || std::is_enum_v<T>, "Generic Write only supports primitive data types");
            ReadBits(&data, size);
        }

        void Read(uint32_t& data, uint32_t size = 32) { ReadBits(&data, size); }
        void Read(int& data, uint32_t size = 32) { ReadBits(&data, size); }
        void Read(float& data) { ReadBits(&data, 32); }
        
        void Read(uint16_t& data, uint32_t size = 16) { ReadBits(&data, size); }
        void Read(int16_t& data, uint32_t size = 16) { ReadBits(&data, size); }
        void Read(uint8_t& data, uint32_t size = 8) { ReadBits(&data, size); }
        
        void Read(bool& data) { ReadBits(&data, 1); }
        void Read(glm::vec2& data);
        void Read(std::string& data);
        
        char* GetBuffer() const { return reinterpret_cast<char*>(_buffer->data()); }
        uint32_t GetRemainingBitCount() const { return _capacity - _head; }
        void ResetToCapacity(uint32_t size) {_capacity = size << 3; _head = 0; }

    private:
        std::shared_ptr<std::vector<uint8_t>> _buffer;
        uint32_t _head;
        uint32_t _capacity;
        
    };
}
