#pragma once

#include "scene.hpp"
#include "gizmos.hpp"

class EditorScene : public Scene
{
private:
	Scene loadedScene{}; //current loaded scene data

	// ===== GAMEOBJECT =====
	int selectedGameObjectIndex = -1; // -1 for no selection

	GizmoAxis activeAxis = GizmoAxis::NONE;
	GizmoMode currentMode = GizmoMode::TRANSLATE;

	float2 dragOffset{ 0.f, 0.f };
	float startMouseAngle = 0.0f;
	float startObjectRotation = 0.0f;

	float2 startMousePos{ 0.f, 0.f };
	float2 startObjectScale{ 1.f, 1.f };

	float2 mouseWorld{};
	bool isMouseDown{};
	bool isMousePressed{};

	void ReadInput();

	void RefreshScene();
	void RefreshRenderers();
	void RefreshColliders();
	void RefreshRigidBodies();
	
	void BuildDockSpace();
	void BuildMenuBar();
	void BuildSceneHierarchyWindow();


	void BuildInspectorWindow();
	void DrawUI();

	void Gizmos();

public:
	//temporary boolean here...
	bool imguiInitialized = false;

	void OnEnter() override;
	void OnUpdate() override;
	void OnExit() override;

	EditorScene() { _name = "Editor"; }
};