#include "ObjectStateReader.h"

#include "ByteStream.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "LeaderboardsUIWindow.h"
#include "PlayerController.h"
#include "Server.h"

void ObjectStateReader::UpdatePlayerList(const char* buffer)
{
	int index = BYTE_STREAM_OVERHEAD;

	std::vector<ClientObject> players;

	const int playerCount = (uint8)buffer[index++];
	for (int i = 0; i < playerCount; i++)
	{
		ClientObject object;
		char nickname[MAX_NICKNAME_SIZE];
		uint8 ID;

		memcpy(&nickname, &buffer[index], MAX_NICKNAME_SIZE);
		index += MAX_NICKNAME_SIZE;

		memcpy(&ID, &buffer[index], sizeof(uint8));
		index += sizeof(uint8);

		object.nickname = nickname;
		object.playerID = ID;

		players.push_back(object);
	}

	LeaderboardsUIWindow::players = players;
}

void ObjectStateReader::SpawnPlayer(const char* buffer)
{
	int index = BYTE_STREAM_OVERHEAD;

	GameObjectManager* objectManager = GameObjectManager::GetInstance();
	const uint8 playerID = buffer[index++];
	const uint8 instanceID = buffer[index++];

	const auto player = objectManager->CreateGameObject(PLAYER_PREFAB_ID);
	if (player)
	{
		const auto controller = player->GetComponent<PlayerController>();
		controller->InitController(playerID);
		player->instanceID = instanceID;
	}
}

void ObjectStateReader::WorldState(const char* buffer)
{
	int index = BYTE_STREAM_OVERHEAD;

	const int count = (uint8)buffer[index++];
	for (int i = 0; i < count; i++)
		ObjectState(buffer, index);
}

void ObjectStateReader::UpdateObject(const char* buffer)
{
	int index = BYTE_STREAM_OVERHEAD;
	ObjectState(buffer, index);
}

void ObjectStateReader::DestroyObject(const char* buffer)
{
	int index = BYTE_STREAM_OVERHEAD;
	uint8 instanceID = buffer[index];

	GameObjectManager* objectManager = GameObjectManager::GetInstance();
	objectManager->GetGameObjectByInstanceID(instanceID)->Destroy();
}

/*
 *		Helpers
 */

void ObjectStateReader::ObjectState(const char* buffer, int& index)
{
	// Game Object ID's
	const uint8 prefabID = buffer[index++];
	const uint8 instanceID = buffer[index++];

	// Owner Game Object
	const uint8 ownerInstanceID = buffer[index++];

	// Position
	int16 x, y;
	memcpy(&x, &buffer[index], sizeof(int16));
	index += sizeof(int16);

	memcpy(&y, &buffer[index], sizeof(int16));
	index += sizeof(int16);

	// Rotation
	int16 rot;
	memcpy(&rot, &buffer[index], sizeof(int16));
	index += sizeof(int16);

	GameObjectManager* objectManager = GameObjectManager::GetInstance();
	const auto found = objectManager->GetGameObjectByInstanceID(instanceID);
	GameObject* go = found ? found : CreateObject(buffer, index, prefabID, instanceID);

	go->owner = objectManager->GetGameObjectByInstanceID(ownerInstanceID);
	go->SetPosition(Vector2D(x, y));
	go->transform.rotation = rot;
}

GameObject* ObjectStateReader::CreateObject(const char* buffer, int& index, const uint8 prefabID, const uint8 instanceID)
{
	GameObjectManager* objectManager = GameObjectManager::GetInstance();
	const auto newObject = objectManager->CreateGameObject(prefabID);
	if(newObject)
	{
		newObject->instanceID = instanceID;

		if (prefabID == PLAYER_PREFAB_ID)
		{
			const uint8 playerID = buffer[index++];
			const auto controller = newObject->GetComponent<PlayerController>();
			controller->InitController(playerID);
		}
	}

	return newObject;
}