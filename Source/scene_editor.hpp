/*
Author: Yan Chun
Co-Author: Nil
*/
#pragma once

#include "scene.hpp"
#include "gizmos.hpp"

class EditorScene : public Scene
{
private:
	Scene loadedScene{}; //current loaded scene data

public:
	void OnEnter() override;
	void OnUpdate() override;
	void OnExit() override;
	bool IsEditorScene() const override { return true; }
	void RefreshScene();

	EditorScene() { _name = "Editor"; }
};