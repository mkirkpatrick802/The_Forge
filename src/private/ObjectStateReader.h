#pragma once
#include "GameData.h"

class GameObject;

class ObjectStateReader
{
public:

	static void SpawnPlayer(const char* buffer);
	static void WorldState(const char* buffer);
	static void UpdateObject(const char* buffer);

private:

	static void ObjectState(const char* buffer, int& index);
	static GameObject* CreateObject(const char* buffer, int& index, uint8 prefabID, uint8 instanceID);

};
