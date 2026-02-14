#pragma once

#include "scene.hpp"

struct EditorScene : Scene
{
private:
	Scene loadedScene{}; //current loaded scene data

	// ===== GAMEOBJECT =====
	int selectedGameObjectIndex = -1; // -1 for no selection
	
	void RefreshRenderers();
	void RefreshColliders();
	void RefreshRigidBodies();
	
	void BuildDockSpace();
	void BuildMenuBar();
	void BuildSceneHierarchyWindow();

	// For UI selection
	int _uiIndex{};
	const char* _uiTypes[2] = { "Display", "Button" };
	const char* _previewType = _uiTypes[_uiIndex];

	void BuildInspectorWindow();
	void DrawUI();

public:
	//temporary boolean here...
	bool imguiInitialized = false;

	void OnEnter() override;
	void OnUpdate() override;
	void OnExit() override;

	EditorScene();
};