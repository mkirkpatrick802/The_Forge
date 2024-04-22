#pragma once
#include "ByteStream.h"
#include "NetcodeUtilites.h"

class GameObject;

class ObjectStateWriter
{
public:

	// Spawns new player
	static ByteStream SpawnPlayer(uint8 playerID);

	// Process Client movement request and returns the players object state
	static ByteStream Movement(const char* buffer);

	// Returns the current world state of replicated objects
	static ByteStream WorldState();

private:

	// Returns a object state
	static ByteStream ObjectState(const GameObject* go, ByteStream& stream);

	// Returns a component state
	static ByteStream ComponentState(uint8 componentID, ByteStream& stream);
};
