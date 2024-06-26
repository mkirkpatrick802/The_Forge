#pragma once
#include "GameData.h"

// Game State Messages from the server to clients
enum class GSM_Server : char
{
	GSM_SpawnPlayer = 0,
	GSM_WorldState,
	GSM_UpdateObject,
	GSM_UpdatePlayerList,
	GSM_DestroyObject
};

// Game State Messages from the client to server
enum class GSM_Client : char
{
	GSM_WorldStateRequest = 0,
	GSM_MovementRequest,
	GSM_FireRequest,
	GSM_UpdateObjectRequest
};

const char BYTE_STREAM_CODE = CHAR_MAX;
const char SERVER_MESSAGE = 0;
const char CLIENT_MESSAGE = 1;

class NetcodeUtilites
{
public:
    static char* GetIPv4Address();
};