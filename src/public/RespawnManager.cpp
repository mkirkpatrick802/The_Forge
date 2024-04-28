#include "RespawnManager.h"

#include "Client.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "ObjectStateWriter.h"
#include "PlayerController.h"

RespawnManager* RespawnManager::_instance = nullptr;

RespawnManager* RespawnManager::GetInstance()
{
	_instance = _instance == nullptr ? new RespawnManager() : _instance;
	return _instance;
}

RespawnManager::RespawnManager() = default;

void RespawnManager::Update(const float deltaTime)
{
	if(_respawningObjects.empty()) return;

    auto it = _respawningObjects.begin();
    while (it != _respawningObjects.end()) 
    {
        auto& object = *it;
        object.respawnTimeRemaining -= deltaTime;
        if (object.respawnTimeRemaining <= 0) 
        {
            Respawn(object);
            it = _respawningObjects.erase(it);
        }
        else 
        {
            ++it;
        }
    }
}

void RespawnManager::StartRespawnTimer(const GameObject* go, const float time)
{
	if(!Client::IsHostClient()) return;

	RespawnObject object;

	object.prefabID = go->GetPrefabID();
	object.instanceID = go->instanceID;

	if (auto controller = go->GetComponent<PlayerController>())
		object.playerID = controller->playerID;

    object.position = go->GetPosition();

	object.respawnTimeRemaining = time;
	_respawningObjects.push_back(object);
}

void RespawnManager::Respawn(const RespawnObject object) const
{
    // Create new object
    GameObjectManager* manager = GameObjectManager::GetInstance();
    GameObject* newObject = manager->CreateGameObject(object.prefabID, object.position);
    newObject->instanceID = object.instanceID;

    // Set player ID if applicable
    if (auto controller = newObject->GetComponent<PlayerController>())
        controller->playerID = object.playerID;

    ObjectStateWriter::UpdateObjectState(newObject, true);
}

void RespawnManager::CleanUp()
{
	delete _instance;
	_instance = nullptr;
}
