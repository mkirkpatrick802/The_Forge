#include "ObjectStateReader.h"

#include "ByteStream.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "PlayerController.h"

void ObjectStateReader::SpawnPlayer(const char* buffer)
{
	int index = BYTE_STREAM_OVERHEAD;

	GameObjectManager* objectManager = GameObjectManager::GetInstance();
	const uint8 playerID = buffer[index++];
	const uint8 instanceID = buffer[index++];

	const auto player = objectManager->CreateGameObject(PLAYER_PREFAB_ID);
	const auto controller = player->GetComponent<PlayerController>();
	controller->InitController(playerID);
	player->instanceID = instanceID;
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

/*
 *		Helpers
 */

void ObjectStateReader::ObjectState(const char* buffer, int& index)
{
	// Game Object ID's
	const uint8 prefabID = buffer[index++];
	const uint8 instanceID = buffer[index++];

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

	go->SetPosition(Vector2D(x, y));
	go->transform.rotation = rot;
}

GameObject* ObjectStateReader::CreateObject(const char* buffer, int& index, const uint8 prefabID, const uint8 instanceID)
{
	GameObjectManager* objectManager = GameObjectManager::GetInstance();
	const auto newObject = objectManager->CreateGameObject(prefabID);
	newObject->instanceID = instanceID;

	if (prefabID == PLAYER_PREFAB_ID)
	{
		const uint8 playerID = buffer[index++];
		const auto controller = newObject->GetComponent<PlayerController>();
		controller->InitController(playerID);
	}

	return newObject;
}