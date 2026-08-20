#pragma once
#include "Engine/UI/UIScreen.h"

// The title screen: a logo, the game's name, and a way into a match.
//
// Content rather than engine. Everything it is built from -- UIImage, UIText, UIButton,
// the canvas and the screen stack -- lives in the engine; what the buttons say and where
// Play goes is Project Steel's business.
class MainMenuScreen final : public Engine::UIScreen
{
public:
    static constexpr const char* SCREEN_NAME = "Main Menu";

    MainMenuScreen() : UIScreen(SCREEN_NAME) {}

    void OnEnter() override;

    // Covers everything. There is no world behind the menu worth showing -- the menu
    // level is deliberately empty -- and nothing behind it should take a click.
    bool IsTransparent() const override { return false; }
    bool IsInputTransparent() const override { return false; }
};
