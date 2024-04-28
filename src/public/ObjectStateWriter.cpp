#include "ObjectStateWriter.h"

#include "GameObject.h"
#include "GameObjectManager.h"
#include "Health.h"
#include "LeaderboardsUIWindow.h"
#include "PlayerController.h"
#include "ScoreManager.h"
#include "Server.h"

ByteStream ObjectStateWriter::UpdatePlayerList(const std::vector<ClientObject>& players)
{
	LeaderboardsUIWindow::players = players;

	ByteStream stream;
	stream.WriteGSM(GSM_Server::GSM_UpdatePlayerList);
	stream.AppendToBuffer((uint8)players.size());
	for (const auto& player : players)
	{
		stream.AppendStringToBuffer(player.nickname, MAX_NICKNAME_SIZE);
		stream.AppendToBuffer(player.playerID);
	}

	return stream;
}

ByteStream ObjectStateWriter::SpawnPlayer(const uint8 playerID)
{
	GameObjectManager* objectManager = GameObjectManager::GetInstance();
	const uint8 instanceID = objectManager->GenerateUniqueInstanceID();

	GameObject* player = objectManager->CreateGameObject(PLAYER_PREFAB_ID);
	if(player)
	{
		const auto controller = player->GetComponent<PlayerController>();
		controller->InitController(playerID);

		player->instanceID = instanceID;

		ScoreManager::SortScore();

		ByteStream stream;
		stream.WriteGSM(GSM_Server::GSM_SpawnPlayer);
		stream.AppendToBuffer(playerID);
		stream.AppendToBuffer(instanceID);
		return stream;
	}

	return {};
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
	if(projectile)
	{
		projectile->instanceID = instanceID;

		GameObject* owner = objectManager->GetGameObjectByInstanceID(buffer[index]);

		projectile->owner = owner;
		projectile->SetPosition(owner->GetPosition());
		projectile->transform.rotation = owner->transform.rotation;

		return UpdateObjectState(projectile);
	}

	return {};
}

ByteStream ObjectStateWriter::RemovePlayer(const uint8 playerID)
{
	GameObjectManager* objectManager = GameObjectManager::GetInstance();
	GameObject* player = objectManager->GetGameObjectByPlayerID(playerID);
	if (!player) return {};

	uint8 instanceID = player->instanceID;
	player->SetShouldRespawn(false);
	player->Destroy();

	ByteStream stream;
	stream.WriteGSM(GSM_Server::GSM_DestroyObject);
	stream.AppendToBuffer(instanceID);
	return stream;
}

void ObjectStateWriter::DestroyObject(const uint8 instanceID)
{
	ByteStream stream;
	stream.WriteGSM(GSM_Server::GSM_DestroyObject);
	stream.AppendToBuffer(instanceID);

	Server::GetServer()->SendByteSteamToAllClients(stream);
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

ByteStream ObjectStateWriter::UpdateObjectRequest(const char* buffer)
{
	const int index = BYTE_STREAM_OVERHEAD;

	GameObjectManager* objectManager = GameObjectManager::GetInstance();
	GameObject* go = objectManager->GetGameObjectByInstanceID(buffer[index]);

	ByteStream stream = UpdateObjectState(go);
	return stream;
}

ByteStream ObjectStateWriter::UpdateObjectState(const GameObject* go, const bool sendStream)
{
	if (go == nullptr) return {};

	ByteStream stream;
	stream.WriteGSM(GSM_Server::GSM_UpdateObject);
	ObjectState(go, stream);

	if (sendStream)
		Server::GetServer()->SendByteSteamToAllClients(stream);

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

	// Owner Instance ID
	uint8 ownerID = go->owner == nullptr ? -1 : go->owner->instanceID;
	stream.AppendToBuffer(ownerID);

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
	{
		stream.AppendToBuffer(go->GetComponent<PlayerController>()->playerID);
		stream.AppendToBuffer((uint8)ScoreManager::GetScore(go->GetComponent<PlayerController>()->playerID));
	}

	if (auto health = go->GetComponent<Health>())
	{
		stream.AppendToBuffer((uint16)health->GetCurrentHealth());
	}

	return stream;
}

ByteStream ObjectStateWriter::ComponentState(const uint8 componentID, ByteStream& stream)
{
	return {};
}