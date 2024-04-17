#pragma once
#include "NetcodeUtilites.h"

class ByteStream
{
public:

	void WriteGSM(GSM_Server message);
	void WriteGSM(GSM_Client message);

	char* GetByteStream() const { return buffer; }

private:
	char* buffer = nullptr;

};