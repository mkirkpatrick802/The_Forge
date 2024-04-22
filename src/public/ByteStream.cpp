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

void ByteStream::WriteSpawnPlayerMessage(int8 playerID)
{
	if ((GSM_Server)buffer[1] != GSM_Server::GSM_SpawnPlayer)
		assert(1);

	buffer[size++] = playerID; // Add player ID
	WriteSpawnObjectMessage();
}

void ByteStream::WriteSpawnObjectMessage()
{
	if ((GSM_Server)buffer[1] != GSM_Server::GSM_SpawnPlayer || (GSM_Server)buffer[1] != GSM_Server::GSM_SpawnObject)
		assert(1);


}

void ByteStream::WriteWorldState()
{
	if ((GSM_Server)buffer[1] != GSM_Server::GSM_WorldState)
		assert(1);

	const int num = GameObjectManager::GetNumOfReplicatedObjects();
	buffer[size++] = (char)num;	// 3

	char state[MAX_GAMEOBJECTS * GAMEOBJECT_STATE_SIZE];
	GameObjectManager::CreateWorldState(state);
	std::memcpy(&buffer[size], &state, sizeof(char) * num * GAMEOBJECT_STATE_SIZE);
	size += sizeof(char) * num * GAMEOBJECT_STATE_SIZE;
}

void ByteStream::WriteObjectState(const uint8 ID)
{
	if ((GSM_Server)buffer[1] != GSM_Server::GSM_UpdateObject)
		assert(1);

	char state[GAMEOBJECT_STATE_SIZE];
	const auto go = GameObjectManager::GetGameObjectByInstanceID(ID);
	GameObjectManager::CreateObjectState(go, state);
	std::memcpy(&buffer[size], &state, sizeof(char) * GAMEOBJECT_STATE_SIZE);
	size += sizeof(char) * GAMEOBJECT_STATE_SIZE;
}

// Send ByteStream from Client to Server
void ByteStream::WriteGSM(const GSM_Client message)
{
	size = 0;
	buffer[size++] = BYTE_STREAM_CODE;	// 0
	buffer[size++] = CLIENT_MESSAGE;	// 1
	buffer[size++] = (char)message;		// 2
}

void ByteStream::WritePlayerMovementRequest(uint8 ID, int8 x, int8 y)
{
	if ((GSM_Client)buffer[1] != GSM_Client::GSM_MovementRequest)
		assert(1);

	buffer[size++] = (char)ID;	// 3
	buffer[size++] = x;			// 4
	buffer[size++] = y;			// 5
}