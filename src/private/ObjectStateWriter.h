#pragma once
#include <vector>

#include "ByteStream.h"
#include "NetcodeUtilites.h"

struct ClientObject;
class GameObject;

class ObjectStateWriter
{
public:

	// Update Player List
	static ByteStream UpdatePlayerList(const std::vector<ClientObject>& players);

	// Spawns new player
	static ByteStream SpawnPlayer(uint8 playerID);

	// Returns the current world state of replicated objects
	static ByteStream WorldState();

	// Update Objects State
	static ByteStream UpdateObjectState(const GameObject* go);

	// Process Client movement request and returns the players object state
	static ByteStream Movement(const char* buffer);

	// Spawn a projectile and tells the clients to do the same
	static ByteStream FireProjectile(const char* buffer);

	// Remove player from game
	static ByteStream RemovePlayer(uint8 playerID);

private:

	// Returns a object state
	static ByteStream ObjectState(const GameObject* go, ByteStream& stream);

	// Returns a component state
	static ByteStream ComponentState(uint8 componentID, ByteStream& stream);
};
