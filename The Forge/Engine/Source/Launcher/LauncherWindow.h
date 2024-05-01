#pragma once
#include <string>

#include "Engine/Data.h"
#include "Engine/UIWindow.h"

enum class Mode
{
	Play,
	Edit
};

struct LauncherSettings
{
	Mode engine = Mode::Play;
	std::string address{ "127.0.0.1" };
	bool isHosting = true;
	bool playButtonPressed = false;
};

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

	LauncherSettings* _settings;
};