#pragma once
#include "Utilites/NetObject.h"

namespace NetCode
{
	class Server final : public NetObject
	{
	public:
		Server();
		~Server() override;
	};
}
