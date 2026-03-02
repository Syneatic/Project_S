#pragma once

#include "scene.hpp"
#include "gizmos.hpp"

class EditorScene : public Scene
{
private:
	Scene loadedScene{}; //current loaded scene data

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

	void Gizmos();

public:
	//temporary boolean here...
	bool imguiInitialized = false;

	void OnEnter() override;
	void OnUpdate() override;
	void OnExit() override;

	void RefreshScene();

	EditorScene() { _name = "Editor"; }
};