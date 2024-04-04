#pragma once
#include "EventDispatcher.h"

class UIWindow : public EventDispatcher
{
public:

	virtual void Render() = 0;

};
