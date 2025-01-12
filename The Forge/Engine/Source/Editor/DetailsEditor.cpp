#include "DetailsEditor.h"

#include "Engine/Component.h"
#include "Engine/ComponentUtils.h"
#include "Engine/GameObject.h"
#include "Engine/Level.h"

Engine::GameObject* Editor::DetailsEditor::_selectedGameObject = nullptr;

void Editor::DetailsEditor::Render()
{
    if (!_selectedGameObject) return;
    
    const String title = _selectedGameObject->GetName() + " Details";
    ImGui::Begin(title.c_str());
    
    for (const auto component : _selectedGameObject->GetAllComponents())
    {
        // Fetch the name using ID and display it
        const uint32 id = component->GetComponentID();
        const String& name = Engine::Component::GetComponentNameFromID(id);
        ImGui::Text(name.c_str());
    }
    
    ImGui::Separator();
    
    if (ImGui::Button("Add Component"))
    {
        ImGui::OpenPopup("Add Component");
    }

    if (ImGui::BeginPopup("Add Component"))
    {
        ImGui::Text("Components");
        ImGui::Separator();

        // Dropdown items
        if (ImGui::Selectable("Sprite Renderer")) { Engine::ComponentUtils::AddComponent(_selectedGameObject, SPRITE_RENDERER); }

        ImGui::EndPopup();
    }
    
    ImGui::End();
}

void Editor::DetailsEditor::SetSelectedGameObject(Engine::GameObject* go)
{
    _selectedGameObject = go;
}
