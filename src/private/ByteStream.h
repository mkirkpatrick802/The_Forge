#pragma once
#include "NetcodeUtilites.h"

class ByteStream
{
public:

	ByteStream();

	void WriteGSM(GSM_Server message);
	void WriteGSM(GSM_Client message);

	void CreatePlayerIDBuffer(uint8 ID);

	char buffer[1024];
	int size;
};