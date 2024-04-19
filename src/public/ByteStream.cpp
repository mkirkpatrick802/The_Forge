//
// Created by mKirkpatrick on 2/24/2024.
//

#include "ByteStream.h"

#include "GameObjectManager.h"

ByteStream::ByteStream(): buffer{}, size(0)
{

}

// Send ByteStream from Server to Clients
void ByteStream::WriteGSM(const GSM_Server message)
{
	buffer[0] = BYTE_STREAM_CODE;
	buffer[1] = SERVER_MESSAGE;
	buffer[2] = (char)message;

	switch (message)
	{
	case GSM_Server::GSM_WorldState:

		char state[MAX_GAMEOBJECTS * GAMEOBJECT_STATE_SIZE];
		GameObjectManager::CreateWorldState(state);
		std::memcpy(&buffer[3], &state, sizeof(char) * (static_cast<uint64>(MAX_GAMEOBJECTS) * GAMEOBJECT_STATE_SIZE));

		break;
	default: ;
	}

	size = (int)std::size(buffer);
}

// Send ByteStream from Client to Server
void ByteStream::WriteGSM(const GSM_Client message)
{
	buffer[0] = BYTE_STREAM_CODE;
	buffer[1] = CLIENT_MESSAGE;
	buffer[2] = (char)message;

	size = (int)std::size(buffer);
}
