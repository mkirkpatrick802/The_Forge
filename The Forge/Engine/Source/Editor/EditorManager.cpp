#include "EditorManager.h"

#include "DetailsEditor.h"
#include "LevelEditor.h"
#include "SceneDisplay.h"
#include "CommandTerminal.h"
#include "Console.h"
#include "ContentDrawer.h"
#include "EditorCamera.h"
#include "EditorShortcuts.h"
#include "MenuBar.h"
#include "Engine/Rendering/UIManager.h"

Editor::EditorManager::EditorManager()
{
    EditorWindows[LEVEL_EDITOR_INDEX] = std::make_shared<LevelEditor>();
    EditorWindows[DETAILS_EDITOR_INDEX] = std::make_shared<DetailsEditor>();
    EditorWindows[SCENE_DISPLAY] = std::make_shared<SceneDisplay>();
    EditorWindows[COMMAND_TERMINAL] = std::make_shared<CommandTerminal>();
    EditorWindows[CONSOLE] = std::make_shared<Console>();
    EditorWindows[CONTENT_DRAWER] = std::make_shared<ContentDrawer>();
    EditorWindows[MENU_BAR] = std::make_shared<MenuBar>();
    
    Engine::UIManager::SetDockingEnabled(true);
    Engine::UIManager::AddUIWindow(EditorWindows[LEVEL_EDITOR_INDEX]);
    Engine::UIManager::AddUIWindow(EditorWindows[DETAILS_EDITOR_INDEX]);
    Engine::UIManager::AddUIWindow(EditorWindows[SCENE_DISPLAY]);
    Engine::UIManager::AddUIWindow(EditorWindows[COMMAND_TERMINAL]);
    Engine::UIManager::AddUIWindow(EditorWindows[CONSOLE]);
    Engine::UIManager::AddUIWindow(EditorWindows[CONTENT_DRAWER]);
    Engine::UIManager::AddUIWindow(EditorWindows[MENU_BAR]);
}

Editor::EditorManager::~EditorManager()
{
    Engine::UIManager::SetDockingEnabled(false);
    Engine::UIManager::RemoveUIWindow(EditorWindows[LEVEL_EDITOR_INDEX]);
    Engine::UIManager::RemoveUIWindow(EditorWindows[DETAILS_EDITOR_INDEX]);
    Engine::UIManager::RemoveUIWindow(EditorWindows[SCENE_DISPLAY]);
    Engine::UIManager::RemoveUIWindow(EditorWindows[COMMAND_TERMINAL]);
    Engine::UIManager::RemoveUIWindow(EditorWindows[CONSOLE]);
    Engine::UIManager::RemoveUIWindow(EditorWindows[CONTENT_DRAWER]);
    Engine::UIManager::RemoveUIWindow(EditorWindows[MENU_BAR]);
    EditorCamera::CleanUp();

    // For some reason IMGUi SetCursor does not work here
    const HCURSOR defaultCursor = LoadCursor(nullptr, IDC_ARROW);
    SetCursor(defaultCursor);
}

void Editor::EditorManager::Update()
{
    EditorShortcuts::GetInstance().Update();
}
