//
// Created by mKirkpatrick on 2/27/2024.
//

#include "EditorEngine.h"

#include <SDL_mouse.h>
#include <SDL_timer.h>
#include <SDL_scancode.h>
#include "GameObject.h"
#include "InputManager.h"
#include "GameObjectManager.h"
#include "UIManager.h"

EditorEngine::EditorEngine(Renderer &renderer, InputManager &inputManager, UIManager &uiManager) : Engine(renderer, inputManager) {
    _uiManager = &uiManager;
    _selectedGameObjectSettings = nullptr;
}

void EditorEngine::GameLoop() {

    // Gameplay Loop
    while (_inputManager->StartProcessInputs(*_renderer)) {
        if(SDL_GetTicks64() - frameStart >= 16) {

            //Frame Rate Management
            deltaTime = (SDL_GetTicks64() - frameStart);
            frameStart = SDL_GetTicks64();

            // Display frame rate in console
            if(_inputManager->GetKey(SDL_SCANCODE_APOSTROPHE))
                printf("%f FPS \n", 1000 / deltaTime);

            // Check if player is clicking game objects
            //ClickObject();

            // Update game objects if game is not in editor mode
            if(!_editorSettings.editMode)
                _gameObjectManager->Update(deltaTime);

            // Render game objects and UI
            _uiManager->Render(_editorSettings, _selectedGameObjectSettings);
            _renderer->Render();

            // End input poll
            _inputManager->EndProcessInputs();

            if(!_editorSettings.editMode) {
                _selectedGameObjectSettings = nullptr;
            }

            if(_editorSettings.editModeChanged) {
                _editorSettings.editModeChanged = false;
                _gameObjectManager->ToggleEditorMode(_editorSettings.editMode);
            }
        }
    }
}

void EditorEngine::ClickObject() {

    Vector2D mouseClickPosition = Vector2D();
    if(_inputManager->GetButtonDown(SDL_BUTTON_LEFT, mouseClickPosition) && _editorSettings.editMode) {

        if(_uiManager->HoveringUI()) return;
        mouseClickPosition = Renderer::ConvertScreenToWorld(mouseClickPosition);
        auto clickedObjects = _gameObjectManager->GetClickedObjects(mouseClickPosition);
        if (!clickedObjects->empty()) {

            auto go = _renderer->GetTopGameObject(  *clickedObjects);
            if(_selectedGameObjectSettings != &go->settings)
                UIManager::ClearFrame();

            _selectedGameObjectSettings = &go->settings;
        }

        delete clickedObjects;
    }
}

void EditorEngine::CleanUp() {
    if(_editorSettings.editMode)
        _gameObjectManager->SaveGameObjectInfo();

    _gameObjectManager->CleanUp();
    delete _gameObjectManager;
}
