#pragma once
#include <string>
#include <unordered_map>

using PrefabID = int;
using PrefabPath = std::string;

const int PLAYER_PREFAB_ID = 0;

class PrefabManager
{
public:

	static PrefabManager& GetInstance()
	{
		static PrefabManager instance;
		return instance;
	}

	std::string GetPrefabPath(const PrefabID id)
	{
		if (const auto it = _prefabPaths.find(id); it != _prefabPaths.end()) 
			return it->second;

		return "";
	}

private:

	PrefabManager()
	{
		_prefabPaths[PLAYER_PREFAB_ID] = "assets/game-data/prefabs/player.json";
	}

	std::unordered_map<PrefabID, PrefabPath> _prefabPaths;
};