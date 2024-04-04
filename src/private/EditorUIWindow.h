#pragma once
#include "UIStructs.h"
#include "UIWindow.h"

class EditorUIWindow : public UIWindow
{
public:

	virtual void Render() override;

	void SetSelectedGameObject(GameObjectSettings* selectedGameObject);
	EditorSettings GetEditorSettings() const;

private:

	Details _details = Details();
	bool _playerSpawned = false;

};