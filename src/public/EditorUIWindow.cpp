#include "EditorUIWindow.h"

#include "SpawnPlayerEvent.h"
#include "DetailsChangedEvent.h"
#include "imgui.h"

void EditorUIWindow::Render()
{
	ImGui::Begin("Menu");

	if (ImGui::CollapsingHeader("Editor Settings"))
	{
		if (ImGui::Checkbox("Edit Mode", &_details.editorSettings.editMode))
		{
			const auto event = CreateEvent<DetailsChangedEvent>();
			event->currentDetails = _details;
			Notify(event);

			if (!_details.editorSettings.editMode)
			{
				_playerSpawned = false;
			}
		}
	}

	if (ImGui::CollapsingHeader("Player Settings"))
	{
		if (ImGui::Button("Spawn Player"))
		{
			if (!_playerSpawned)
			{
				const auto event = CreateEvent<SpawnPlayerEvent>();
				Notify(event);
			}

			_playerSpawned = true;
		}
	}

	if (_details.gameObjectSettings != nullptr)
	{
		ImGui::Text(" ");
		ImGui::Text("%s", _details.gameObjectSettings->name->c_str());

		if (ImGui::CollapsingHeader("Game Object Settings"))
		{
			std::string currentName = *_details.gameObjectSettings->name;
			char buf[256];
			strncpy_s(buf, currentName.c_str(), sizeof(buf));
			buf[sizeof(buf) - 1] = 0;

			if (ImGui::InputText("Name", buf, sizeof(buf)))
			{
				currentName = std::string(buf);
				*_details.gameObjectSettings->name = currentName;
			}

			float position[2] = { _details.gameObjectSettings->position->x, _details.gameObjectSettings->position->y };
			ImGui::InputFloat2("Position", position);
			*_details.gameObjectSettings->position = Vector2D(position[0], position[1]);
		}
	}
}

void EditorUIWindow::SetSelectedGameObject(GameObjectSettings* selectedGameObject)
{
	_details.gameObjectSettings = selectedGameObject;
}

EditorSettings EditorUIWindow::GetEditorSettings() const
{
	return _details.editorSettings;
}
