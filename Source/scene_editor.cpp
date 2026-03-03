
//systems
#include "renderer.hpp"
#include "physics.hpp"
#include "gameobject.hpp"
#include "audio.hpp"
#include "camera.hpp"
#include "particle.hpp"

//scene
#include "scene_parser.hpp"
#include "scene_editor.hpp"
#include "editor_imgui.hpp"

//comps
#include "components.hpp"


namespace
{

}

void EditorScene::ReadInput()
{
	s32 mX, mY;
	AEInputGetCursorPosition(&mX, &mY);
	f32 camX, camY;
	AEGfxGetCamPosition(&camX, &camY);

	mouseWorld.x = camX + (((float)mX - AEGfxGetWindowWidth() * 0.5f) / CameraData::zoomMult);
	mouseWorld.y = camY + ((AEGfxGetWindowHeight() * 0.5f - (float)mY) / CameraData::zoomMult);

	isMouseDown = AEInputCheckCurr(AEVK_LBUTTON);
	isMousePressed = AEInputCheckTriggered(AEVK_LBUTTON);

	if (AEInputCheckTriggered(AEVK_Q)) currentMode = GizmoMode::TRANSLATE;
	if (AEInputCheckTriggered(AEVK_W)) currentMode = GizmoMode::ROTATE;
	if (AEInputCheckTriggered(AEVK_E)) currentMode = GizmoMode::SCALE;
}

void EditorScene::RefreshScene()
{
	Editor::selectedObjects.clear(); //reset index selection
	CameraSystem::OnStart();
}

void EditorScene::Gizmos() {
	if (Editor::selectedObjects.empty()) return;

	GameObject& selectedObj = *Editor::selectedObjects[0];
	Transform& trans = selectedObj.transform();

	//avg position
	float2 position{};

	//draw for each selected object
	for (const auto& obj : Editor::selectedObjects)
	{
		//draw for children as well
		Graphics::RenderData outline;
		outline.alignment = Graphics::Alignment::MC;
		outline.blendMode = Graphics::BlendMode::AE_GFX_BM_NONE;
		outline.drawMode = Graphics::DrawMode::AE_GFX_MDM_LINES;
		outline.color = Color(0xFF'FC'67'3A);
		outline.layer = (Graphics::RenderLayer)(Graphics::RenderLayer::GIZMOS + 25);
		outline.pos = obj->transform().position;
		outline.scale = obj->transform().scale;
		outline.rot = obj->transform().rotation;
		position += obj->transform().position;
		Graphics::Submit(outline, Graphics::PrimitiveType::BOX);
	}

	position /= Editor::selectedObjects.size();

	// 1. Draw the active gizmo
	switch (currentMode) {
	case GizmoMode::TRANSLATE: DrawTranslationGizmo(position); break;
	case GizmoMode::ROTATE:    DrawRotationGizmo(position); break;
	case GizmoMode::SCALE:     DrawScaleGizmo(position); break;
	}

	// 2. Handle Interaction
	if (isMousePressed) {
		activeAxis = GetHitAxis(mouseWorld, trans.position);

		if (currentMode == GizmoMode::ROTATE && activeAxis == GizmoAxis::ROTATION) {
			startMouseAngle = atan2f(mouseWorld.y - trans.position.y, mouseWorld.x - trans.position.x);
			startObjectRotation = trans.rotation;
		}
		else if (currentMode == GizmoMode::SCALE) {
			startMousePos = mouseWorld;
			startObjectScale = trans.scale;
		}
		else {
			dragOffset = trans.position - mouseWorld;
		}
	}

	//moving
	if (isMouseDown && activeAxis != GizmoAxis::NONE) {
		if (currentMode == GizmoMode::TRANSLATE) {
			if (activeAxis == GizmoAxis::X || activeAxis == GizmoAxis::CENTER)
				trans.position.x = mouseWorld.x + dragOffset.x;
			if (activeAxis == GizmoAxis::Y || activeAxis == GizmoAxis::CENTER)
				trans.position.y = mouseWorld.y + dragOffset.y;
		}
		else if (currentMode == GizmoMode::ROTATE) {
			float currentAngle = atan2f(mouseWorld.y - trans.position.y, mouseWorld.x - trans.position.x);
			trans.rotation = startObjectRotation + (currentAngle - startMouseAngle) * (180.0f / 3.14159f);
		}
		else if (currentMode == GizmoMode::SCALE) {
			float2 delta = mouseWorld - startMousePos;
			if (activeAxis == GizmoAxis::X) trans.scale.x = startObjectScale.x + delta.x;
			if (activeAxis == GizmoAxis::Y) trans.scale.y = startObjectScale.y + delta.y;
			if (activeAxis == GizmoAxis::CENTER)
			{
				float factor = 1.0f + (delta.x / 100.0f);
				trans.scale = startObjectScale * factor;
			}
		}
	}
	else if (!isMouseDown) {
		activeAxis = GizmoAxis::NONE;
	}
}

void EditorScene::OnEnter()
{
	RefreshScene();
	ParticleSystem::Initialize();
	CameraSystem::OnStart();
}

void EditorScene::OnUpdate()
{
	ReadInput();
	CameraSystem::OnUpdate(); // Check input and update camera matrix

	AEGfxSetBackgroundColor(0.f, 0.f, 0.f);
	//bool imguiFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);

	//draw gizmos last
	Gizmos(); //gizmos execution

	//uniquely for editor only
	for (auto& pgo : loadedScene.gameObjectList())
	{
		auto* go = pgo.get();
		go->UpdateWorldTransform();

		for (auto& [type, comp] : go->componentMap())
		{
			if (auto* c = dynamic_cast<Renderer*>(comp.get()))
			{
				c->OnUpdate();
			}

			if (auto* c = dynamic_cast<ParticleEmitter*>(comp.get()))
			{
				c->OnUpdate();
			}
		}
	}
	ParticleSystem::Update();

	Graphics::Execute();
	//we shall simulate particles too
	ParticleSystem::Render();

	//draw imgui after game render
	if (imguiInitialized)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		Editor::DrawUI(*this,loadedScene);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGui::EndFrame();
	}

}

void EditorScene::OnExit() 
{
	//unload everything
	Graphics::Flush();
	Physics::FlushColliders();
	Physics::FlushRigidBody();
	CameraSystem::OnExit();
}
