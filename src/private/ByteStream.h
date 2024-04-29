#pragma once
#include "NetcodeUtilites.h"

const int BYTE_STREAM_OVERHEAD = 3;
const int MAX_STREAM_SIZE = 200;

class ByteStream
{
public:

	ByteStream();

	void WriteGSM(GSM_Server message);
	
	void WriteGSM(GSM_Client message);
	void WritePlayerMovementRequest(uint8 ID, int8 x, int8 y);
	void WriteFireRequest(uint8 ID);

	template <typename T>
	void AppendToBuffer(T value);

	void AppendStringToBuffer(const std::string& str, int strSize);

	char buffer[MAX_STREAM_SIZE];
	int size;
};

template <typename T>
void ByteStream::AppendToBuffer(T value)
{
	static_assert(std::is_integral_v<T>, "Only integer types are allowed.");

	if (size + sizeof(T) > MAX_STREAM_SIZE) assert(0 && "Buffer overflow prevented");

	std::memcpy(&buffer[size], &value, sizeof(T));
	size += sizeof(T);
}