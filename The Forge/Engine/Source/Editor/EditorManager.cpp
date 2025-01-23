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
    EditorWindows[LEVEL_EDITOR_INDEX] = std::make_shared<LevelEditor>();
    EditorWindows[DETAILS_EDITOR_INDEX] = std::make_shared<DetailsEditor>();
    EditorWindows[SCENE_DISPLAY] = std::make_shared<SceneDisplay>();
    EditorWindows[COMMAND_TERMINAL] = std::make_shared<CommandTerminal>();
    EditorWindows[CONSOLE] = std::make_shared<Console>();
    
    Engine::UIManager::SetDockingEnabled(true);
    Engine::UIManager::AddUIWindow(EditorWindows[LEVEL_EDITOR_INDEX]);
    Engine::UIManager::AddUIWindow(EditorWindows[DETAILS_EDITOR_INDEX]);
    Engine::UIManager::AddUIWindow(EditorWindows[SCENE_DISPLAY]);
    Engine::UIManager::AddUIWindow(EditorWindows[COMMAND_TERMINAL]);
    Engine::UIManager::AddUIWindow(EditorWindows[CONSOLE]);
}

Editor::EditorManager::~EditorManager()
{
    Engine::UIManager::SetDockingEnabled(false);
    Engine::UIManager::RemoveAllUIWindows();
    EditorCamera::CleanUp();
}