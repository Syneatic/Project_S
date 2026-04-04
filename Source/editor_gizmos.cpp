/*
Author: Yan Chun
Co-Author: Nil
*/
//systems
#include "physics.hpp"
#include "gameobject.hpp"
#include "audio.hpp"
#include "camera.hpp"

//scene
#include "scene_parser.hpp"
#include "scene_editor.hpp"
#include "imgui_helper.hpp"
#include "editor.hpp"

//comps
#include "components.hpp"

namespace //helpers
{
	GizmoAxis activeAxis = GizmoAxis::NONE;
	GizmoMode currentMode = GizmoMode::TRANSLATE;

	// Per-object drag state — indexed parallel to selectedObjects at drag start
	struct ObjectDragState
	{
		float2 startWorldPos{};
		float  startWorldRot{};
		float2 startWorldScale{};
	};
	std::vector<ObjectDragState> dragStates{};

	float2 startMousePos{};
	float  startMouseAngle{};

	float2 mouseWorld{};
	bool   isMouseDown{};
	bool   isMousePressed{};

	// Write a new world position back to an object, converting to local if needed.
	void SetWorldPosition(GameObject& obj, float2 newWorldPos)
	{
		if (obj.parent())
		{
			const Transform& pw = obj.parent()->worldTransform();
			float2 local{};
			local.x = pw.scale.x != 0.f ? (newWorldPos.x - pw.position.x) / pw.scale.x
				: newWorldPos.x - pw.position.x;
			local.y = pw.scale.y != 0.f ? (newWorldPos.y - pw.position.y) / pw.scale.y
				: newWorldPos.y - pw.position.y;
			obj.transform().position = local;
		}
		else
			obj.transform().position = newWorldPos;
	}

	void SetWorldRotation(GameObject& obj, float newWorldRot)
	{
		if (obj.parent())
			obj.transform().rotation = newWorldRot - obj.parent()->worldTransform().rotation;
		else
			obj.transform().rotation = newWorldRot;
	}

	void SetWorldScale(GameObject& obj, float2 newWorldScale)
	{
		if (obj.parent())
		{
			const Transform& pw = obj.parent()->worldTransform();
			obj.transform().scale.x = pw.scale.x != 0.f ? newWorldScale.x / pw.scale.x : newWorldScale.x;
			obj.transform().scale.y = pw.scale.y != 0.f ? newWorldScale.y / pw.scale.y : newWorldScale.y;
		}
		else
			obj.transform().scale = newWorldScale;
	}

	Graphics::RenderData GetRenderData(const Transform& t)
	{
		Graphics::RenderData outline;
		outline.alignment = Graphics::Alignment::MC;
		outline.blendMode = Graphics::BlendMode::AE_GFX_BM_NONE;
		outline.drawMode = Graphics::DrawMode::AE_GFX_MDM_LINES;
		outline.color = Color(0xFF'FC'67'3A);
		outline.layer = (Graphics::RenderLayer)(Graphics::RenderLayer::GIZMOS + 25);
		outline.pos = t.position;
		outline.scale = t.scale;
		outline.rot = t.rotation;
		return outline;
	}
}

namespace Editor
{
	void ReadGizmosInput()
	{
		s32 mX, mY;
		AEInputGetCursorPosition(&mX, &mY);
		mouseWorld = CameraSystem::ScreenToWorld({ static_cast<f32>(mX), static_cast<f32>(mY) });

		isMouseDown = AEInputCheckCurr(AEVK_LBUTTON);
		isMousePressed = AEInputCheckTriggered(AEVK_LBUTTON);

		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow)) return;
		if (AEInputCheckTriggered(AEVK_Q)) currentMode = GizmoMode::TRANSLATE;
		if (AEInputCheckTriggered(AEVK_W)) currentMode = GizmoMode::ROTATE;
		if (AEInputCheckTriggered(AEVK_E)) currentMode = GizmoMode::SCALE;
	}

	void DrawGizmos()
	{
		ReadGizmosInput();
		if (Editor::selectedObjects.empty()) return;

		// ---- Draw selection outlines + compute shared pivot ----
		float2 pivot{};
		for (const auto& obj : Editor::selectedObjects)
		{
			auto outline = GetRenderData(obj->worldTransform());
			pivot += obj->worldTransform().position;

			Graphics::Submit(outline, Graphics::PrimitiveType::BOX);

			for (auto& child : obj->children())
			{
				auto coutline = GetRenderData(child.get()->worldTransform());
				Graphics::Submit(coutline, Graphics::PrimitiveType::BOX);
			}
		}
		pivot /= (float)Editor::selectedObjects.size();

		// ---- Draw gizmo at shared pivot ----
		switch (currentMode)
		{
		case GizmoMode::TRANSLATE: DrawTranslationGizmo(pivot); break;
		case GizmoMode::ROTATE:    DrawRotationGizmo(pivot);    break;
		case GizmoMode::SCALE:     DrawScaleGizmo(pivot);       break;
		}

		// ---- On press: record per-object start state ----
		if (isMousePressed)
		{
			activeAxis = GetHitAxis(mouseWorld, pivot);

			if (activeAxis != GizmoAxis::NONE)
			{
				startMousePos = mouseWorld;
				startMouseAngle = atan2f(mouseWorld.y - pivot.y, mouseWorld.x - pivot.x);

				dragStates.clear();
				for (const auto& obj : Editor::selectedObjects)
				{
					ObjectDragState s;
					s.startWorldPos = obj->worldTransform().position;
					s.startWorldRot = obj->worldTransform().rotation;
					s.startWorldScale = obj->worldTransform().scale;
					dragStates.push_back(s);
				}
			}
		}

		// ---- While held: apply transform in world space, convert back to local ----
		if (isMouseDown && activeAxis != GizmoAxis::NONE)
		{
			float2 mouseDelta = mouseWorld - startMousePos;

			for (int i = 0; i < (int)Editor::selectedObjects.size(); i++)
			{
				GameObject& obj = *Editor::selectedObjects[i];
				const auto& start = dragStates[i];

				if (currentMode == GizmoMode::TRANSLATE)
				{
					float2 newWorld = start.startWorldPos;
					if (activeAxis == GizmoAxis::X || activeAxis == GizmoAxis::CENTER)
						newWorld.x += mouseDelta.x;
					if (activeAxis == GizmoAxis::Y || activeAxis == GizmoAxis::CENTER)
						newWorld.y += mouseDelta.y;
					SetWorldPosition(obj, newWorld);
				}
				else if (currentMode == GizmoMode::ROTATE)
				{
					float currentAngle = atan2f(mouseWorld.y - pivot.y, mouseWorld.x - pivot.x);
					float deltaRad = currentAngle - startMouseAngle;
					float deltaDeg = deltaRad * (180.f / 3.14159f);

					// Rotate each object's world position around the shared pivot
					float2 offset = start.startWorldPos - pivot;
					float  cosA = cosf(deltaRad);
					float  sinA = sinf(deltaRad);
					float2 rotatedOffset{
						offset.x * cosA - offset.y * sinA,
						offset.x * sinA + offset.y * cosA
					};
					SetWorldPosition(obj, pivot + rotatedOffset);
					SetWorldRotation(obj, start.startWorldRot + deltaDeg);
				}
				else if (currentMode == GizmoMode::SCALE)
				{
					float2 newScale = start.startWorldScale;
					if (activeAxis == GizmoAxis::X)
						newScale.x = start.startWorldScale.x + mouseDelta.x;
					if (activeAxis == GizmoAxis::Y)
						newScale.y = start.startWorldScale.y + mouseDelta.y;
					if (activeAxis == GizmoAxis::CENTER)
					{
						float factor = 1.f + (mouseDelta.x / 100.f);
						newScale = start.startWorldScale * factor;
					}

					// Also offset position to keep objects anchored relative to pivot
					float2 offset = start.startWorldPos - pivot;
					float2 scaleFactor{
						start.startWorldScale.x != 0.f ? newScale.x / start.startWorldScale.x : 1.f,
						start.startWorldScale.y != 0.f ? newScale.y / start.startWorldScale.y : 1.f
					};
					float2 newWorldPos = pivot + float2{ offset.x * scaleFactor.x, offset.y * scaleFactor.y };
					SetWorldPosition(obj, newWorldPos);
					SetWorldScale(obj, newScale);
				}
			}
		}
		else if (!isMouseDown)
			activeAxis = GizmoAxis::NONE;
	}
}
