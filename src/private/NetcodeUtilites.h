#pragma once
#include "GameData.h"

// Game State Messages from the server to clients
enum class GSM_Server : char
{
	GSM_SpawnPlayer = 0,
	GSM_WorldState,
	GSM_SendPlayerID // Should not be used with WriteGSM functions
};

// Game State Messages from the client to server
enum class GSM_Client : char
{
	GSM_SyncWorld = 0,
	GSM_MovementInput,
};

const char BYTE_STREAM_CODE = CHAR_MAX;
const char SERVER_MESSAGE = 0;
const char CLIENT_MESSAGE = 1;

class NetcodeUtilites
{
public:
    static char* GetIPv4Address();
};