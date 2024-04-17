#include "EditorEngine.h"

#include <SDL_mouse.h>
#include <SDL_timer.h>
#include <SDL_scancode.h>

#include "DetailsChangedEvent.h"
#include "GameObject.h"
#include "InputManager.h"
#include "GameObjectManager.h"
#include "UIManager.h"

EditorEngine::EditorEngine(Renderer& renderer, InputManager& inputManager) :
	Engine(renderer, inputManager)
{
	_editorUI.Attach(_gameObjectManager);
	_editorUI.Attach(this);

	_uiManager->AddUIWindow(&_editorUI);
}

void EditorEngine::GameLoop()
{
	// Gameplay Loop
	while (_inputManager->StartProcessInputs(*_renderer)) 
	{
		if (SDL_GetTicks64() - frameStart >= 16) 
		{
			//Frame Rate Management
			deltaTime = (SDL_GetTicks64() - frameStart);
			frameStart = SDL_GetTicks64();

			// Display frame rate in console
			if (_inputManager->GetKey(SDL_SCANCODE_APOSTROPHE))
				printf("%f FPS \n", 1000 / deltaTime);

			// Get UI Data
			_details.editorSettings = _editorUI.GetEditorSettings();

			// Check if player is clicking game objects
			ClickObject();

			// Update game objects if game is not in editor mode
			if (!_details.editorSettings.editMode)
				_gameObjectManager->Update(deltaTime);

			// Render game objects and UI
			_uiManager->Render();
			_renderer->Render();

			// End input poll
			_inputManager->EndProcessInputs();
		}
	}
}

void EditorEngine::ClickObject()
{
	if (!_details.editorSettings.editMode)
	{
		_editorUI.SetSelectedGameObject(nullptr);
	}

	Vector2D mouseClickPosition = Vector2D();
	if (_inputManager->GetButtonDown(SDL_BUTTON_LEFT, mouseClickPosition) && _details.editorSettings.editMode)
	{
		if (UIManager::HoveringUI()) return;
		mouseClickPosition = Renderer::ConvertScreenToWorld(mouseClickPosition);
		const auto clickedObjects = _gameObjectManager->GetClickedObjects(mouseClickPosition);
		if (!clickedObjects->empty()) 
		{
			const auto go = _renderer->GetTopGameObject(*clickedObjects);
			if (_details.gameObjectSettings != &go->settings)
				UIManager::ClearFrame();

			_editorUI.SetSelectedGameObject(&go->settings);
		}

		delete clickedObjects;
	}
}

void EditorEngine::OnEvent(Event* event)
{
	switch(event->GetEventType())
	{
	case EventType::ET_NULL:
		break;
	case EventType::ET_SpawnPlayer:
		break;
	case EventType::ET_DetailsChanged:
		_details = static_cast<DetailsChangedEvent*>(event)->currentDetails;
		break;
	}
}

void EditorEngine::CleanUp()
{
	if (_details.editorSettings.editMode)
		_gameObjectManager->SaveGameObjectInfo();

	_gameObjectManager->CleanUp();
	delete _gameObjectManager;
}
