#pragma once
#include "GameData.h"

// Game State Messages from the server to clients
enum class GSM_Server : uint8
{
	GSM_SpawnPlayer = 1,
};

// Game State Messages from the client to server
enum class GSM_Client : uint8
{
	GSM_MovementInput = 1,
};

class NetcodeUtilites
{
public:
    static char* GetIPv4Address();
};