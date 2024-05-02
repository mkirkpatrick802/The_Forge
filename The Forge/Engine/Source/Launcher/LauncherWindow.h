#pragma once

#include "Engine/Data.h"
#include "Engine/UIWindow.h"

struct LauncherSettings;

class LauncherWindow final : public Engine::UIWindow
{
public:

	LauncherWindow();

	void SetLauncherSettings(LauncherSettings* settings) { _settings = settings; }
	virtual void Render() override;

private:

	void DrawMenu();

private:

	unsigned int _iconTexture;
	Vector2D _iconSize;

	LauncherSettings* _settings = nullptr;
};