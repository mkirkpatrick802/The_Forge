#pragma once
#include <string>
#include <unordered_map>

using PrefabID = int;
using PrefabPath = std::string;

const int PLAYER_PREFAB_ID = 0;
const int PROJECTILE_PREFAB_ID = 1;
const int ASTEROID_PREFAB_ID = 2;
const int ENEMY_PREFAB_ID = 3;

class PrefabManager
{
public:

	static PrefabManager& GetInstance()
	{
		static PrefabManager instance;
		return instance;
	}

	std::string GetPrefabPath(const PrefabID ID)
	{
		if (const auto it = _prefabPaths.find(ID); it != _prefabPaths.end())
			return it->second;

		return "";
	}

private:

	PrefabManager()
	{
		_prefabPaths[PLAYER_PREFAB_ID] = "assets/game-data/prefabs/player.json";
		_prefabPaths[PROJECTILE_PREFAB_ID] = "assets/game-data/prefabs/projectile.json";
		_prefabPaths[ASTEROID_PREFAB_ID] = "assets/game-data/prefabs/asteroid.json";
		_prefabPaths[ENEMY_PREFAB_ID] = "assets/game-data/prefabs/enemy.json";
	}

	std::unordered_map<PrefabID, PrefabPath> _prefabPaths;
};