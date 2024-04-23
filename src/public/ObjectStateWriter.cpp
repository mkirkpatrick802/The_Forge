#include "ObjectStateWriter.h"

#include "GameObject.h"
#include "GameObjectManager.h"
#include "PlayerController.h"

ByteStream ObjectStateWriter::SpawnPlayer(const uint8 playerID)
{
	GameObjectManager* objectManager = GameObjectManager::GetInstance();
	const uint8 instanceID = objectManager->GenerateUniqueInstanceID();

	GameObject* player = objectManager->CreateGameObject(PLAYER_PREFAB_ID);
	const auto controller = player->GetComponent<PlayerController>();
	controller->InitController(playerID);

	player->instanceID = instanceID;

	ByteStream stream;
	stream.WriteGSM(GSM_Server::GSM_SpawnPlayer);
	stream.AppendToBuffer(playerID);
	stream.AppendToBuffer(instanceID);
	return stream;
}

ByteStream ObjectStateWriter::Movement(const char* buffer)
{
	int index = BYTE_STREAM_OVERHEAD;

	const uint8 instanceID = buffer[index++];
	const int8 xAxis = buffer[index++];
	const int8 yAxis = buffer[index++];

	GameObjectManager* objectManager = GameObjectManager::GetInstance();
	if (const auto go = objectManager->GetGameObjectByInstanceID(instanceID))
	{
		float speed = 0;
		if (go->GetComponent<PlayerController>())
			speed = go->GetComponent<PlayerController>()->movementSpeed;

		const Vector2D movementVector = normalize(Vector2D((float)xAxis,(float)yAxis * -1)) * speed;
		go->SetRotationWithVector(Vector2D(xAxis, yAxis), 90);
		go->SetPosition(movementVector + go->GetPosition());

		ByteStream stream;
		stream.WriteGSM(GSM_Server::GSM_UpdateObject);
		ObjectState(go, stream);
		return stream;
	}

	assert(0 && "Invalid Movement Request");
	return {};
}

ByteStream ObjectStateWriter::FireProjectile(const char* buffer)
{
	const int index = BYTE_STREAM_OVERHEAD;

	GameObjectManager* objectManager = GameObjectManager::GetInstance();
	const uint8 instanceID = objectManager->GenerateUniqueInstanceID();

	GameObject* projectile = objectManager->CreateGameObject(PROJECTILE_PREFAB_ID);
	projectile->instanceID = instanceID;

	const GameObject* owner = objectManager->GetGameObjectByInstanceID(buffer[index]);

	projectile->SetPosition(owner->GetPosition());
	projectile->transform.rotation = owner->transform.rotation;

	ByteStream stream;
	stream.WriteGSM(GSM_Server::GSM_UpdateObject);
	ObjectState(projectile, stream);
	return stream;
}

ByteStream ObjectStateWriter::WorldState()
{
	const GameObjectManager* objectManager = GameObjectManager::GetInstance();
	std::vector<GameObject*> replicatedObjects;
	for (const auto go : objectManager->GetCurrentGameObjects())
		if (go->isReplicated)
			replicatedObjects.push_back(go);

	ByteStream stream;
	stream.WriteGSM(GSM_Server::GSM_WorldState);

	const uint8 count = (uint8)replicatedObjects.size();
	stream.AppendToBuffer(count);

	for (const auto go : replicatedObjects)
		ObjectState(go, stream);

	return stream;
}

/*
 *		Helpers
 */

ByteStream ObjectStateWriter::ObjectState(const GameObject* go, ByteStream& stream)
{
	// Prefab ID
	stream.AppendToBuffer(go->GetPrefabID());

	// Instance ID
	stream.AppendToBuffer(go->instanceID);

	// Write Position
	const int16 x = (int16)go->GetPosition().x;
	const int16 y = (int16)go->GetPosition().y;

	stream.AppendToBuffer(x);
	stream.AppendToBuffer(y);

	// Write rotation
	const int16 rot = (int16)std::round(go->transform.rotation);
	stream.AppendToBuffer(rot);

	// Player ID
	if (go->GetPrefabID() == PLAYER_PREFAB_ID) // TODO: Use Component State Function
		stream.AppendToBuffer(go->GetComponent<PlayerController>()->playerID);

	return stream;
}

ByteStream ObjectStateWriter::ComponentState(const uint8 componentID, ByteStream& stream)
{
	return {};
}