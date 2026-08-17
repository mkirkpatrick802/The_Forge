#pragma once
#include <glm/vec2.hpp>

#include "Engine/Rendering/UIWindow.h"

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
	glm::vec2 _iconSize;

	LauncherSettings* _settings = nullptr;
};