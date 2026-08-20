#include "DetailsEditor.h"

#include "Engine/GameObject.h"
#include "Engine/Components/Component.h"
#include "Engine/Components/ComponentRegistry.h"
#include "Engine/Components/ComponentFactories.h"

Engine::GameObject* Editor::DetailsEditor::_selectedGameObject = nullptr;

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
    _selectedGameObject = go;
}

void Editor::DetailsEditor::ClearSelectedGameObject()
{
    _selectedGameObject = nullptr;
}
