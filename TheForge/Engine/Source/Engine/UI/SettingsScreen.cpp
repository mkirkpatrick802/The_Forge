#include "SettingsScreen.h"

#include <SDL_video.h>

#include "UIButton.h"
#include "UIImage.h"
#include "UIRoot.h"
#include "UISlider.h"
#include "UIText.h"
#include "Engine/EngineManager.h"
#include "Engine/LaunchOptions.h"
#include "Engine/System.h"

namespace
{
    const std::string SETTINGS_FILE = "settings.json";

    const std::string KEY_WIDTH = "Window Width";
    const std::string KEY_HEIGHT = "Window Height";
    const std::string KEY_FULLSCREEN = "Fullscreen";
    const std::string KEY_VOLUME = "Master Volume";

    // Cyclers rather than a free-form list: a fixed set of 16:9 sizes covers what this
    // engine's 320x180 reference resolution scales to cleanly, and a cycler needs no
    // dropdown widget -- which does not exist yet.
    struct Resolution { int width, height; };

    const Resolution RESOLUTIONS[] = {
        {1280, 720}, {1600, 900}, {1920, 1080}, {2560, 1440},
    };

    constexpr int RESOLUTION_COUNT = static_cast<int>(std::size(RESOLUTIONS));

    // The stored resolution's index, or the closest one below it. Falls back to the
    // first entry, so an unrecognised or absent size is a sane window rather than none.
    int FindResolutionIndex(const int width, const int height)
    {
        for (int i = 0; i < RESOLUTION_COUNT; ++i)
            if (RESOLUTIONS[i].width == width && RESOLUTIONS[i].height == height) return i;

        return 0;
    }

    std::string ReadString(const std::string& key, const std::string& fallback)
    {
        const auto value = Engine::GetEngineManager().GetConfigData(SETTINGS_FILE, key);

        // Everything goes through UpdateConfigFile, which writes strings -- so read them
        // back as strings rather than trusting a type the file might not have.
        if (value.is_string()) return value.get<std::string>();
        if (value.is_number()) return std::to_string(value.get<double>());

        return fallback;
    }

    float ReadFloat(const std::string& key, const float fallback)
    {
        try { return std::stof(ReadString(key, std::to_string(fallback))); }
        catch (...) { return fallback; }
    }

    int ReadInt(const std::string& key, const int fallback)
    {
        try { return std::stoi(ReadString(key, std::to_string(fallback))); }
        catch (...) { return fallback; }
    }
}

Engine::SettingsScreen::SettingsScreen()
    : UIScreen(SCREEN_NAME)
{
}

Engine::SettingsScreen::~SettingsScreen() = default;

float Engine::SettingsScreen::GetMasterVolume()
{
    return ReadFloat(KEY_VOLUME, 1.0f);
}

void Engine::SettingsScreen::ApplyStoredSettings()
{
    // A dedicated server has no window to size and no volume to set.
    if (GetLaunchOptions().headless) return;

    SDL_Window* window = GetAppWindow();
    if (window == nullptr) return;

    const int width = ReadInt(KEY_WIDTH, 0);
    const int height = ReadInt(KEY_HEIGHT, 0);
    const bool fullscreen = ReadInt(KEY_FULLSCREEN, 0) != 0;

    // Fullscreen first, then the size. Setting a size while fullscreen is ignored by
    // SDL, so doing it the other way round silently drops the resolution change.
    SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);

    if (!fullscreen && width > 0 && height > 0)
    {
        SDL_SetWindowSize(window, width, height);
        SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }
}

void Engine::SettingsScreen::OnEnter()
{
    _canvas.Clear();
    _canvas.SetBlocksInput(true);

    const glm::vec2 canvasSize = _canvas.GetCanvasSize();

    const int width = ReadInt(KEY_WIDTH, 1280);
    const int height = ReadInt(KEY_HEIGHT, 720);
    _resolutionIndex = FindResolutionIndex(width, height);
    _fullscreen = ReadInt(KEY_FULLSCREEN, 0) != 0;
    _volume = ReadFloat(KEY_VOLUME, 1.0f);

    // Nearly opaque. Settings opens over the pause menu, and at 0.92 the pause menu's
    // own PAUSED / RESUME / SETTINGS text read straight through the settings text sitting
    // on top of it -- two menus legible at once, which is worse than either. Left just
    // short of solid so it still reads as a layer over the game rather than a new place.
    const auto shade = _canvas.Add<UIImage>();
    shade->Rect().size = canvasSize;
    shade->SetTint(glm::vec3(0.03f, 0.04f, 0.06f));
    shade->SetOpacity(0.985f);

    const auto title = _canvas.Add<UIText>("SETTINGS", 16);
    title->Rect().anchor = EUIAnchor::TopCenter;
    title->Rect().offset = glm::vec2(0.0f, 20.0f);
    title->Rect().size = glm::vec2(canvasSize.x, 16.0f);
    title->Rect().pivot = glm::vec2(0.5f, 0.0f);
    title->SetAlign(EUITextAlign::Center);

    _resolutionButton = _canvas.Add<UIButton>("", [this] { CycleResolution(1); });
    _resolutionButton->Rect().anchor = EUIAnchor::TopCenter;
    _resolutionButton->Rect().offset = glm::vec2(0.0f, 54.0f);
    _resolutionButton->Rect().pivot = glm::vec2(0.5f, 0.0f);
    _resolutionButton->Rect().size = glm::vec2(150.0f, 14.0f);
    _resolutionButton->Label().SetFontSize(9);

    _fullscreenButton = _canvas.Add<UIButton>("", [this] { ToggleFullscreen(); });
    _fullscreenButton->Rect().anchor = EUIAnchor::TopCenter;
    _fullscreenButton->Rect().offset = glm::vec2(0.0f, 72.0f);
    _fullscreenButton->Rect().pivot = glm::vec2(0.5f, 0.0f);
    _fullscreenButton->Rect().size = glm::vec2(150.0f, 14.0f);
    _fullscreenButton->Label().SetFontSize(9);

    _volumeLabel = _canvas.Add<UIText>("", 9);
    _volumeLabel->Rect().anchor = EUIAnchor::TopCenter;
    _volumeLabel->Rect().offset = glm::vec2(0.0f, 92.0f);
    _volumeLabel->Rect().size = glm::vec2(canvasSize.x, 9.0f);
    _volumeLabel->Rect().pivot = glm::vec2(0.5f, 0.0f);
    _volumeLabel->SetAlign(EUITextAlign::Center);

    _volumeSlider = _canvas.Add<UISlider>();
    _volumeSlider->Rect().anchor = EUIAnchor::TopCenter;
    _volumeSlider->Rect().offset = glm::vec2(-75.0f, 108.0f);
    _volumeSlider->Rect().size = glm::vec2(150.0f, 4.0f);
    _volumeSlider->SetRange(0.0f, 1.0f);
    _volumeSlider->SetValue(_volume);
    _volumeSlider->SetOnChanged([this](const float value)
    {
        _volume = value;
        GetEngineManager().UpdateConfigFile(SETTINGS_FILE, KEY_VOLUME, std::to_string(value));
        RefreshLabels();
    });

    const auto back = _canvas.Add<UIButton>("BACK", [] { UIRoot::Pop(); });
    back->Rect().anchor = EUIAnchor::BottomCenter;
    back->Rect().offset = glm::vec2(0.0f, -14.0f);
    back->Rect().pivot = glm::vec2(0.5f, 1.0f);
    back->Rect().size = glm::vec2(90.0f, 14.0f);
    back->Label().SetFontSize(9);

    const auto note = _canvas.Add<UIText>("volume is stored but has nothing to drive yet -- there is no audio system", 5);
    note->Rect().anchor = EUIAnchor::BottomCenter;
    note->Rect().offset = glm::vec2(0.0f, -4.0f);
    note->Rect().size = glm::vec2(canvasSize.x - 20.0f, 5.0f);
    note->Rect().pivot = glm::vec2(0.5f, 1.0f);
    note->SetAlign(EUITextAlign::Center);
    note->SetColor(glm::vec3(0.4f, 0.42f, 0.46f));
    note->SetWrap(true);

    RefreshLabels();
}

void Engine::SettingsScreen::RefreshLabels()
{
    if (_resolutionButton != nullptr)
    {
        const Resolution& resolution = RESOLUTIONS[_resolutionIndex];
        _resolutionButton->SetLabel("RESOLUTION: " + std::to_string(resolution.width) + "x" + std::to_string(resolution.height));

        // Meaningless while fullscreen, and saying so beats letting someone change a
        // number that visibly does nothing.
        _resolutionButton->SetEnabled(!_fullscreen);
    }

    if (_fullscreenButton != nullptr)
        _fullscreenButton->SetLabel(_fullscreen ? "DISPLAY: FULLSCREEN" : "DISPLAY: WINDOWED");

    if (_volumeLabel != nullptr)
        _volumeLabel->SetText("MASTER VOLUME: " + std::to_string(static_cast<int>(_volume * 100.0f + 0.5f)) + "%");
}

void Engine::SettingsScreen::CycleResolution(const int direction)
{
    _resolutionIndex = (_resolutionIndex + direction + RESOLUTION_COUNT) % RESOLUTION_COUNT;

    const Resolution& resolution = RESOLUTIONS[_resolutionIndex];
    GetEngineManager().UpdateConfigFile(SETTINGS_FILE, KEY_WIDTH, std::to_string(resolution.width));
    GetEngineManager().UpdateConfigFile(SETTINGS_FILE, KEY_HEIGHT, std::to_string(resolution.height));

    ApplyDisplay();
    RefreshLabels();
}

void Engine::SettingsScreen::ToggleFullscreen()
{
    _fullscreen = !_fullscreen;
    GetEngineManager().UpdateConfigFile(SETTINGS_FILE, KEY_FULLSCREEN, std::to_string(_fullscreen ? 1 : 0));

    ApplyDisplay();
    RefreshLabels();
}

void Engine::SettingsScreen::ApplyDisplay() const
{
    // Read back through the same path startup uses, rather than applying the members
    // directly -- so there is one implementation of "make the window match the config"
    // and no chance of the two drifting.
    ApplyStoredSettings();
}
