#include "DetailsEditor.h"

#include <fstream>

#include "Engine/GameObject.h"
#include "Engine/Components/Component.h"
#include "Engine/Components/ComponentRegistry.h"
#include "Engine/Components/ComponentFactories.h"

Engine::GameObject* Editor::DetailsEditor::_selectedGameObject = nullptr;
std::unique_ptr<Engine::GameObject> Editor::DetailsEditor::_selectedPrefab = nullptr;

Editor::DetailsEditor::~DetailsEditor()
{
    
}

void Editor::DetailsEditor::Render()
{
    if (!_selectedGameObject) return;
    
    ImGui::Begin("Details Panel");
    
    ImGui::TextUnformatted(_selectedGameObject->GetName().c_str());
    ImGui::Spacing();
    ImGui::PushItemWidth(50);
    ImGui::Text("Position:");
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10); // Small padding
    ImGui::InputFloat("##X", &_selectedGameObject->transform.position.x, 0, 0);
    ImGui::SameLine();
    ImGui::InputFloat("##Y", &_selectedGameObject->transform.position.y, 0, 0);
    ImGui::PopItemWidth();

    ImGui::Spacing();
    ImGui::PushItemWidth(50);
    ImGui::Text("Rotation:");
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10); // Small padding
    ImGui::InputFloat("##Angle", &_selectedGameObject->transform.rotation);
    ImGui::PopItemWidth();

    ImGui::Checkbox("Is Replicated", &_selectedGameObject->isReplicated);
    ImGui::Spacing();
    
    ImGui::Separator();
    
    for (const auto component : _selectedGameObject->GetAllComponents())
    {
        const std::string& name = Engine::GetComponentRegistry().GetComponentName(typeid(*component));
        ImGui::TextUnformatted(name.c_str());
        ImGui::Spacing();

        component->DrawDetails();
        ImGui::Spacing();
        ImGui::Separator();
    }
    
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
    ClearSelectedPrefab();
    
    _selectedGameObject = go;
}

void Editor::DetailsEditor::ClearSelectedGameObject()
{
    ClearSelectedPrefab();
    
    _selectedGameObject = nullptr;
}

void Editor::DetailsEditor::SetSelectedPrefab(std::unique_ptr<Engine::GameObject> prefab)
{
    _selectedPrefab = std::move(prefab);
    _selectedGameObject = _selectedPrefab.get();
}

void Editor::DetailsEditor::ClearSelectedPrefab()
{
    if (_selectedPrefab)
    {
        // Write new data
        const nlohmann::json data = _selectedPrefab->Serialize();
        if (std::ofstream outputFile(_selectedPrefab->filepath); outputFile.is_open())
        {
            outputFile << data.dump(4);
            outputFile.close();
        }
        
        _selectedPrefab.reset();
    }
}
