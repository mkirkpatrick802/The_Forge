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
    ClearSelectedGameObject();
}

void Editor::DetailsEditor::Render()
{
    if (!_selectedGameObject) return;
    
    ImGui::Begin("Details Panel");
    
    ImGui::TextUnformatted(_selectedGameObject->GetName().c_str());
    ImGui::Spacing();

    ImGui::Checkbox("Is Replicated", &_selectedGameObject->isReplicated);
    ImGui::SameLine();
    ImGui::Checkbox("Is Server Only", &_selectedGameObject->isServerOnly);
    ImGui::Spacing();
    
    ImGui::Separator();
    
    for (int i = 0; i < static_cast<int>(_selectedGameObject->GetAllComponents().size()); i++)
    {
        auto component = _selectedGameObject->GetAllComponents()[i];
        const std::string& name = Engine::GetComponentRegistry().GetComponentName(typeid(*component));
        if (ImGui::CollapsingHeader(name.c_str()))
        {
            component->DrawDetails();
            ImGui::Spacing();
            ImGui::Separator();
        }

        if (ImGui::BeginPopupContextItem(name.c_str()))
        {
            if (ImGui::MenuItem("Remove Component"))
            {
                _selectedGameObject->RemoveComponent(component);
            }

            ImGui::EndPopup();
        }
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
