#pragma once
#include "UIWindow.h"
#include "Server.h"

class LeaderboardsUIWindow : public UIWindow
{
public:

	virtual void Render() override;

	static std::vector<ClientObject> players;
};
