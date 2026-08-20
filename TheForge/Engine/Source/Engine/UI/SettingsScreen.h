#pragma once
#include <string>
#include <vector>

#include "UIScreen.h"

namespace Engine
{
    class UIButton;
    class UISlider;
    class UIText;

    // Display and audio options, persisted to Config/settings.json.
    //
    // Written through EngineManager::UpdateConfigFile, the same path the editor uses for
    // its own preferences -- so there is one notion of "a config file" rather than a
    // second one invented here.
    class SettingsScreen final : public UIScreen
    {
    public:
        static constexpr const char* SCREEN_NAME = "Settings";

        SettingsScreen();
        ~SettingsScreen() override;

        void OnEnter() override;

        // Transparent, so opening settings over a pause menu leaves the game visible
        // behind both -- but opaque to input, so a click cannot reach the pause menu
        // underneath.
        bool IsTransparent() const override { return true; }
        bool IsInputTransparent() const override { return false; }

        // Applies what is stored in Config/settings.json to the live window. Called once
        // at startup, so a resolution chosen last session is in effect before anything
        // is drawn.
        static void ApplyStoredSettings();

        // The stored master volume, 0..1. Nothing consumes it yet -- there is no audio
        // system -- but the setting is recorded so that whatever gets written next reads
        // a value the player already chose rather than inventing a default.
        static float GetMasterVolume();

    private:
        void CycleResolution(int direction);
        void ToggleFullscreen();
        void RefreshLabels();

        void ApplyDisplay() const;

    private:
        UIButton* _resolutionButton = nullptr;
        UIButton* _fullscreenButton = nullptr;
        UIText* _volumeLabel = nullptr;
        UISlider* _volumeSlider = nullptr;

        int _resolutionIndex = 0;
        bool _fullscreen = false;
        float _volume = 1.0f;
    };
}
