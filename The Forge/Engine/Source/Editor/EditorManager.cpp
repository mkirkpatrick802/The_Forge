#include "EditorManager.h"

#include "DetailsEditor.h"
#include "LevelEditor.h"
#include "SceneDisplay.h"
#include "CommandTerminal.h"
#include "Console.h"
#include "EditorCamera.h"
#include "Engine/Rendering/UIManager.h"

Editor::EditorManager::EditorManager()
{
    EditorWindows[LEVEL_EDITOR_INDEX] = std::make_unique<LevelEditor>();
    EditorWindows[DETAILS_EDITOR_INDEX] = std::make_unique<DetailsEditor>();
    EditorWindows[SCENE_DISPLAY] = std::make_unique<SceneDisplay>();
    EditorWindows[COMMAND_TERMINAL] = std::make_unique<CommandTerminal>();
    EditorWindows[CONSOLE] = std::make_unique<Console>();
    
    Engine::UIManager::SetDockingEnabled(true);
    Engine::UIManager::AddUIWindow(EditorWindows[LEVEL_EDITOR_INDEX].get());
    Engine::UIManager::AddUIWindow(EditorWindows[DETAILS_EDITOR_INDEX].get());
    Engine::UIManager::AddUIWindow(EditorWindows[SCENE_DISPLAY].get());
    Engine::UIManager::AddUIWindow(EditorWindows[COMMAND_TERMINAL].get());
    Engine::UIManager::AddUIWindow(EditorWindows[CONSOLE].get());
}

Editor::EditorManager::~EditorManager()
{
    Engine::UIManager::SetDockingEnabled(false);
    Engine::UIManager::RemoveAllUIWindows();
    EditorCamera::CleanUp();
}