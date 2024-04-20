#pragma once
#include "NetcodeUtilites.h"

class ByteStream
{
public:

	ByteStream();

	void WriteGSM(GSM_Server message);
	void WriteWorldState();
	void WriteObjectState(uint8 ID);
	
	void WriteGSM(GSM_Client message);
	void WritePlayerMovementRequest(uint8 ID, int8 x, int8 y);

	char buffer[1024];
	int size;
};