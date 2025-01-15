#include "EditorManager.h"

#include "DetailsEditor.h"
#include "LevelEditor.h"
#include "Engine/UIManager.h"

Editor::EditorManager::EditorManager()
{
    EditorWindows[LEVEL_EDITOR_INDEX] = std::make_unique<LevelEditor>();
    EditorWindows[DETAILS_EDITOR_INDEX] = std::make_unique<DetailsEditor>();

    Engine::UIManager::AddUIWindow(EditorWindows[LEVEL_EDITOR_INDEX].get());
    Engine::UIManager::AddUIWindow(EditorWindows[DETAILS_EDITOR_INDEX].get());
}

Editor::EditorManager::~EditorManager()
{
    Engine::UIManager::RemoveUIWindow(EditorWindows[LEVEL_EDITOR_INDEX].get());
    Engine::UIManager::RemoveUIWindow(EditorWindows[DETAILS_EDITOR_INDEX].get());
}
