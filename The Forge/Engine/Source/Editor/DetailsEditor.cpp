#include "DetailsEditor.h"

#include "Engine/Components/Component.h"
#include "Engine/Components/ComponentRegistry.h"
#include "Engine/Components/ComponentFactories.h"

Engine::GameObject* Editor::DetailsEditor::_selectedGameObject = nullptr;

void Editor::DetailsEditor::Render()
{
    if (!_selectedGameObject) return;
    
    const std::string title = _selectedGameObject->GetName() + " Details";
    ImGui::Begin(title.c_str());

    ImGui::PushItemWidth(100);
    ImGui::InputFloat("X Position", &_selectedGameObject->transform.position.x, 1);
    
    ImGui::SameLine();
    
    ImGui::PushItemWidth(100);
    ImGui::InputFloat("Y Position", &_selectedGameObject->transform.position.y, 1);
    
    for (const auto component : _selectedGameObject->GetAllComponents())
    {
        const std::string& name = Engine::GetComponentRegistry().GetComponentName(typeid(*component));
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
        for (const auto& [fst, snd] : Engine::GetComponentRegistry().GetListOfComponents())
            if (ImGui::Selectable(fst.c_str())) { Engine::GetComponentFactories().CreateComponentFromID(snd, _selectedGameObject); }

        ImGui::EndPopup();
    }
    
    ImGui::End();
}

void Editor::DetailsEditor::SetSelectedGameObject(Engine::GameObject* go)
{
    _selectedGameObject = go;
}
