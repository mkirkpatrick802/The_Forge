#pragma once
#include <string>

#include "Engine/Data.h"
#include "Engine/UIWindow.h"
#include "glad/glad.h"

enum class Mode
{
	Play,
	Edit
};

class LauncherWindow final : public Engine::UIWindow
{
public:

	LauncherWindow();
	virtual void Render() override;

private:

	void DrawMenu();

private:

	GLuint iconTexture;
	Vector2D iconSize;

	Mode currentMode = Mode::Play;
	std::string address{"127.0.0.1"};
	bool isHosting = true;
};