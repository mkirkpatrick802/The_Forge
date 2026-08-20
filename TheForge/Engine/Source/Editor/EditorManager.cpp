#include "EditorManager.h"
#include "Engine/Rendering/DebugRenderer.h"
#include "Engine/Components/RectangleCollider.h"
#include "Engine/Components/CircleCollider.h"
#include "Engine/Components/PolygonCollider.h"
#include "Engine/GameObject.h"

#include "DetailsEditor.h"
#include "LevelEditor.h"
#include "SceneDisplay.h"
#include "CommandTerminal.h"
#include "Console.h"
#include "ContentDrawer.h"
#include "EditorCamera.h"
#include "EditorShortcuts.h"
#include "MenuBar.h"
#include "Engine/EngineManager.h"
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
    EditorCamera::GetInstance()->Update();

    // Queued every frame: the debug renderer clears its lists after each draw, so an
    // overlay has to be resubmitted rather than set once.
    DrawSelectionOverlay();
}

void Editor::EditorManager::DrawSelectionOverlay()
{
    const Engine::GameObject* selected = DetailsEditor::GetSelectedGameObject();
    if (selected == nullptr) return;

    constexpr glm::vec3 enabledColour(0.2f, 1.0f, 0.35f);
    constexpr glm::vec3 disabledColour(0.6f, 0.6f, 0.6f);

    for (Engine::Component* component : selected->GetAllComponents())
    {
        const auto collider = dynamic_cast<Engine::Collider*>(component);
        if (collider == nullptr) continue;

        const glm::vec3 colour = collider->GetEnabled() ? enabledColour : disabledColour;
        const glm::vec2 centre = collider->GetCenter();

        // Bounds are axis-aligned and centre-anchored, and deliberately not rotated --
        // that is what the collision tests themselves use, so the outline shows what
        // actually collides rather than what the sprite looks like.
        if (collider->GetColliderType() == Engine::EColliderType::ECT_Circle)
        {
            if (const auto circle = dynamic_cast<Engine::CircleCollider*>(collider))
                Engine::GetDebugRenderer().DrawOverlayCircle(centre, circle->GetRadius(), colour, 24);
        }
        else if (collider->GetColliderType() == Engine::EColliderType::ECT_Rectangle)
        {
            if (const auto rectangle = dynamic_cast<Engine::RectangleCollider*>(collider))
                Engine::GetDebugRenderer().DrawOverlayRectangle(centre, rectangle->GetSize(), colour);
        }
        else if (collider->GetColliderType() == Engine::EColliderType::ECT_Polygon)
        {
            // Piece by piece rather than as one silhouette: the seams are where a bad
            // decomposition shows, and this is the view you have while authoring.
            if (const auto polygon = dynamic_cast<Engine::PolygonCollider*>(collider))
                for (const auto& piece : polygon->GetWorldPieces())
                    Engine::GetDebugRenderer().DrawOverlayPolygon(piece, colour);
        }
    }
}
