#pragma once

#include <vector>

#include "GameData.h"

struct RespawnObject
{
	uint8 prefabID;
	uint8 instanceID;

	uint8 playerID = 255;

	float respawnTimeRemaining;

	Vector2D position;
};

class GameObject;
class RespawnManager
{
public:

	static RespawnManager* GetInstance();

	void Update(float deltaTime);

	void StartRespawnTimer(const GameObject* go, float time);
	void Respawn(RespawnObject object) const;

	static void CleanUp();

private:

	RespawnManager();

	static RespawnManager* _instance;

	std::vector<RespawnObject> _respawningObjects;

};
