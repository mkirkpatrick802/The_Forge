#include "DetailsEditor.h"

#include <fstream>

#include "Engine/GameObject.h"
#include "Engine/Components/Component.h"
#include "Engine/Components/ComponentRegistry.h"
#include "Engine/Components/ComponentFactories.h"

Engine::GameObject* Editor::DetailsEditor::_selectedGameObject = nullptr;

Editor::DetailsEditor::~DetailsEditor()
{
    delete _selectedGameObject;
    _selectedGameObject = nullptr;
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

    ImGui::Checkbox("Is Replicated", &_selectedGameObject->isReplicated);
    ImGui::Spacing();
    
    ImGui::Separator();
    
    for (const auto component : _selectedGameObject->GetAllComponents())
    {
        const std::string& name = Engine::GetComponentRegistry().GetComponentName(typeid(*component));
        ImGui::TextUnformatted(name.c_str());
        ImGui::Spacing();

        component->DrawDetails();
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

void Editor::DetailsEditor::CleanUpPrefab()
{
    if (_selectedGameObject && _selectedGameObject->isPrefab)
    {
        json data = _selectedGameObject->Serialize();
        
        // Write new data
        if (std::ofstream outputFile(_selectedGameObject->filepath); outputFile.is_open())
        {
            outputFile << data.dump(4);
            outputFile.close();
        }
        
        delete _selectedGameObject;
    }
}

void Editor::DetailsEditor::SetSelectedGameObject(Engine::GameObject* go)
{
    CleanUpPrefab();
    
    _selectedGameObject = go;
}

void Editor::DetailsEditor::ClearSelectedGameObject()
{
    _selectedGameObject = nullptr;
}
