#pragma once
#include "NetcodeUtilites.h"

const int BYTE_STREAM_OVERHEAD = 3;

class ByteStream
{
public:

	ByteStream();

	void WriteGSM(GSM_Server message);
	void WriteSpawnPlayerMessage(int8 playerID);
	void WriteSpawnObjectMessage();
	void WriteWorldState();
	void WriteObjectState(uint8 ID);
	
	void WriteGSM(GSM_Client message);
	void WritePlayerMovementRequest(uint8 ID, int8 x, int8 y);

	char buffer[1024];
	int size;
};