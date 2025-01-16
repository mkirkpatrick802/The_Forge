#include "EditorManager.h"

#include "DetailsEditor.h"
#include "LevelEditor.h"
#include "SceneDisplay.h"
#include "Engine/Rendering/UIManager.h"

Editor::EditorManager::EditorManager()
{
    EditorWindows[LEVEL_EDITOR_INDEX] = std::make_unique<LevelEditor>();
    EditorWindows[DETAILS_EDITOR_INDEX] = std::make_unique<DetailsEditor>();
    EditorWindows[SCENE_DISPLAY] = std::make_unique<SceneDisplay>();
    
    Engine::UIManager::SetDockingEnabled(true);
    Engine::UIManager::AddUIWindow(EditorWindows[LEVEL_EDITOR_INDEX].get());
    Engine::UIManager::AddUIWindow(EditorWindows[DETAILS_EDITOR_INDEX].get());
    Engine::UIManager::AddUIWindow(EditorWindows[SCENE_DISPLAY].get());
}

Editor::EditorManager::~EditorManager()
{
    Engine::UIManager::SetDockingEnabled(false);
    Engine::UIManager::RemoveUIWindow(EditorWindows[LEVEL_EDITOR_INDEX].get());
    Engine::UIManager::RemoveUIWindow(EditorWindows[DETAILS_EDITOR_INDEX].get());
    Engine::UIManager::RemoveUIWindow(EditorWindows[SCENE_DISPLAY].get());
}