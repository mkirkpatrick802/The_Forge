//
// Created by mKirkpatrick on 2/24/2024.
//

#include "ByteStream.h"
#include "GameObjectManager.h"

ByteStream::ByteStream(): buffer{}, size(0) {}

// Send ByteStream from Server to Clients
void ByteStream::WriteGSM(const GSM_Server message)
{
	size = 0;
	buffer[size++] = BYTE_STREAM_CODE;  // 0
	buffer[size++] = SERVER_MESSAGE;	// 1
	buffer[size++] = (char)message;		// 2
}

// Send ByteStream from Client to Server
void ByteStream::WriteGSM(const GSM_Client message)
{
	size = 0;
	buffer[size++] = BYTE_STREAM_CODE;	// 0
	buffer[size++] = CLIENT_MESSAGE;	// 1
	buffer[size++] = (char)message;		// 2
}

void ByteStream::WritePlayerMovementRequest(const uint8 ID, const int8 x, const int8 y)
{
	if ((GSM_Client)buffer[2] != GSM_Client::GSM_MovementRequest)
		assert(0);

	buffer[size++] = (char)ID;	// 3
	buffer[size++] = x;			// 4
	buffer[size++] = y;			// 5
}

void ByteStream::WriteFireRequest(const uint8 ID)
{
	if ((GSM_Client)buffer[2] != GSM_Client::GSM_FireRequest)
		assert(0);

	buffer[size++] = (char)ID;
}

void ByteStream::AppendStringToBuffer(const std::string& str, const int strSize)
{
	std::memcpy(&buffer[size], str.c_str(), strSize);
	size += strSize;
}