//
// Created by mKirkpatrick on 2/24/2024.
//

#include "ByteStream.h"

// Send ByteStream from Server to Clients
void ByteStream::WriteGSM(const GSM_Server message)
{
	buffer[0] = BYTE_STREAM_CODE;
	buffer[1] = SERVER_MESSAGE;

	switch (message)
	{
	case GSM_Server::GSM_SpawnPlayer:
		buffer[2] = static_cast<char>(GSM_Server::GSM_SpawnPlayer);
		break;
	}

	size = std::size(buffer);
}

// Send ByteStream from Client to Server
void ByteStream::WriteGSM(const GSM_Client message)
{
	buffer[0] = BYTE_STREAM_CODE;
	buffer[1] = CLIENT_MESSAGE;

	switch (message)
	{

	}

	size = std::size(buffer);
}
