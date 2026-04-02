// systems
#include "renderer.hpp"
#include "physics.hpp"
#include "gameobject.hpp"
#include "audio.hpp"
#include "camera.hpp"
#include "particle.hpp"

// scene
#include "scene_parser.hpp"
#include "scene_debugger.hpp"
#include "imgui_helper.hpp"

// comps
#include "components.hpp"


namespace
{
	void UpdateGO(GameObject& go)
	{
		for (auto& [type, comp] : go.componentMap())
		{
			if (auto* c = dynamic_cast<Renderer*>(comp.get()))
				c->OnUpdate();
			if (auto* c = dynamic_cast<ParticleEmitter*>(comp.get()))
				c->OnUpdate();
		}

		for (auto& child : go.children())
		{
			UpdateGO(*child);
		}
	}

	bool HitTest(const GameObject& go, float2 worldMouse)
	{
		const Transform& wt = go.worldTransform();
		float halfW = std::abs(wt.scale.x) * 0.5;
		float halfH = std::abs(wt.scale.y) * 0.5;

		const float minHalf = 0.1f;
		halfW = std::max(halfW, minHalf);
		halfH = std::max(halfH, minHalf);

		return (worldMouse.x >= wt.position.x - halfW &&
			worldMouse.x <= wt.position.x + halfW &&
			worldMouse.y >= wt.position.y - halfH &&
			worldMouse.y <= wt.position.y + halfH);
	}

	GameObject* PickRecursive(GameObject& go, float2 worldMouse)
	{
		// Test children first (they're drawn on top)
		for (auto& child : go.children())
		{
			if (auto* hit = PickRecursive(*child, worldMouse))
				return hit;
		}
		return HitTest(go, worldMouse) ? &go : nullptr;
	}

	void DrawHoverOutline(const GameObject& go)
	{
		Graphics::RenderData outline{};
		outline.alignment = Graphics::Alignment::MC;
		outline.blendMode = Graphics::BlendMode::AE_GFX_BM_NONE;
		outline.drawMode = Graphics::DrawMode::AE_GFX_MDM_LINES;
		outline.color = Color(0xFF'00'FF'FF);
		outline.layer = (Graphics::RenderLayer)(Graphics::RenderLayer::GIZMOS + 20);
		outline.pos = go.worldTransform().position;
		outline.scale = go.worldTransform().scale;
		outline.rot = go.worldTransform().rotation;
		Graphics::Submit(outline, Graphics::PrimitiveType::BOX);
	}
}

namespace Debugger
{
	GameObject* PickHoveredObject(Scene& scene)
	{
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) || ImGui::GetIO().WantCaptureMouse)
			return nullptr;

		s32 mx, my;
		AEInputGetCursorPosition(&mx, &my);
		float2 worldMouse = CameraSystem::ScreenToWorld({ static_cast<f32>(mx), static_cast<f32>(my) });

		for (auto& pgo : scene.gameObjectList())
		{
			if (auto* hit = PickRecursive(*pgo, worldMouse))
				return hit;
		}
		return nullptr;
	}
}

namespace
{
	void BuildDockSpcae()
	{
		ImGuiWindowFlags host_flags =
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

		const ImGuiViewport* vp = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(vp->WorkPos);
		ImGui::SetNextWindowSize(vp->WorkSize);
		ImGui::SetNextWindowViewport(vp->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		ImGui::Begin("##debugger_dockspace", nullptr, host_flags);
		ImGui::PopStyleVar(2);

		ImGuiID dockspace_id = ImGui::GetID("DebuggerDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::End();
	}

	void BuildMenuBar(DebuggerScene& dscene)
	{
		ImGui::BeginMainMenuBar();

		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.05f, 0.25f, 0.25f, 1.f));

		if (ImGui::BeginMenu("Debugger##menu"))
		{
			ImGui::TextDisabled("Read-only mode");
			ImGui::Separator();

			if (ImGui::MenuItem("Switch to Editor"))
				SceneManager::SwitchToEditor();

			ImGui::Separator();
			if (ImGui::MenuItem("Quit"))
				EngineCTX::applicationRunning = false;

			ImGui::EndMenu();
		}

		{
			std::string label = "[DEBUGGER]  scene: " + dscene.name();
			float textW = ImGui::CalcTextSize(label.c_str()).x;
			float barW = ImGui::GetContentRegionAvail().x;
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (barW - textW) * 0.5f);
			ImGui::TextDisabled("%s", label.c_str());
		}

		ImGui::PopStyleColor();
		ImGui::EndMainMenuBar();
	}

	void DrawNodeReadOnly(GameObject* go, GameObject* hovered)
	{
		bool hasChildren = !go->children().empty();
		bool isHovered = (go == hovered);

		ImGuiTreeNodeFlags flags = 
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_FramePadding |
			ImGuiTreeNodeFlags_DrawLinesToNodes;
		flags |= hasChildren ? 0 : ImGuiTreeNodeFlags_Leaf;

		if (isHovered)
		{
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.f, 0.6f, 0.6f, 0.6f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.f, 0.8f, 0.8f, 0.7f));
		}

		if (go->cname().empty()) return;
		bool nodeOpen = ImGui::TreeNodeEx(go->cname().c_str(), flags);

		if (isHovered)
			ImGui::PopStyleColor(2);

		if (ImGui::IsItemHovered())
		{
			const Transform& wt = go->worldTransform();
			ImGui::SetTooltip("World pos (%.2f, %.2f)\nWorld scale (%.2f, %.2f)\nWorld rot %.2f deg",
				wt.position.x, wt.position.y,
				wt.scale.x, wt.scale.y,
				wt.rotation);
		}

		if (nodeOpen)
		{
			for (auto& child : go->children())
				DrawNodeReadOnly(child.get(), hovered);
			ImGui::TreePop();
		}
	}

	void BuildHierarchyWindow(Scene& scene, GameObject* hovered)
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(280.f, 100.f), ImVec2(FLT_MAX, FLT_MAX));
		ImGui::Begin("Scene (Debugger)##window");

		// Scene name ¨C read-only
		ImGui::TextUnformatted(scene.name().c_str());
		ImGui::Separator();

		for (auto& pgo : scene.gameObjectList())
			DrawNodeReadOnly(pgo.get(), hovered);

		ImGui::End();
	}

	void ReadOnlyFloat2(const char* label, const float2& v)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(label);
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("X: %.3f   Y: %.3f", v.x, v.y);
	}

	void ReadOnlyFloat(const char* label, const float& v)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(label);
		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%.3f", v);
	}

	void ReadOnlyBool(const char* label, bool v)
	{
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(label);
		ImGui::TableSetColumnIndex(1);

		ImGui::BeginDisabled(true);
		bool tmp = v;
		std::string id = std::string("##ro_") + label;
		ImGui::Checkbox(id.c_str(), &tmp);
		ImGui::EndDisabled();
	}

	bool BeginPropertyTable(const char* id)
	{
		return ImGui::BeginTable(id, 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchSame);
	}

	void BuildInspectorWindow(GameObject* hovered)
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(300.f, 100.f), ImVec2(FLT_MAX, FLT_MAX));
		ImGui::Begin("Inspector (Debugger)##window");

		if (!hovered)
		{
			ImGui::TextDisabled("Hover over a GameObject to inspect it.");
			ImGui::End();
			return;
		}

		// ©¤©¤ Header ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 1.f, 1.f, 1.f));
		ImGui::TextUnformatted(hovered->cname().c_str());
		ImGui::PopStyleColor();

		ImGui::SameLine();
		ImGui::TextDisabled("  id: %zu", hovered->id);

		{
			bool active = hovered->active();
			ImGui::BeginDisabled(true);
			ImGui::Checkbox("Active##ro_go", &active);
			ImGui::EndDisabled();
		}

		ImGui::Separator();

		// ©¤©¤ Local Transform ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
		if (ImGui::CollapsingHeader("Transform (local)", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const Transform& lt = hovered->transform();
			if (BeginPropertyTable("##lt_table"))
			{
				ReadOnlyFloat2("Position", lt.position);
				ReadOnlyFloat2("Scale", lt.scale);
				ReadOnlyFloat("Rotation", lt.rotation);
				ImGui::EndTable();
			}
		}

		// ©¤©¤ World Transform ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
		if (ImGui::CollapsingHeader("Transform (world)", ImGuiTreeNodeFlags_DefaultOpen))
		{
			const Transform& wt = hovered->worldTransform();
			if (BeginPropertyTable("##wt_table"))
			{
				ReadOnlyFloat2("Position", wt.position);
				ReadOnlyFloat2("Scale", wt.scale);
				ReadOnlyFloat("Rotation", wt.rotation);
				ImGui::EndTable();
			}
		}

		ImGui::Separator();

		// ©¤©¤ Components ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
		const auto& comps = hovered->componentMap();
		const auto& compOrder = hovered->componentOrder();

		for (const std::type_index& type : compOrder)
		{
			auto it = comps.find(type);
			if (it == comps.end() || !it->second) continue;

			const Component* comp = it->second.get();

			bool open = ImGui::CollapsingHeader(comp->name().c_str(),
				ImGuiTreeNodeFlags_DefaultOpen);

			// Active badge (read-only)
			{
				bool active = comp->active();
				ImGui::SameLine();
				std::string id = "##ro_comp_active_" + comp->name();
				ImGui::BeginDisabled(true);
				ImGui::Checkbox(id.c_str(), &active);
				ImGui::EndDisabled();
			}

			if (open)
			{
				// Wrap the component's own DrawInInspector inside BeginDisabled
				// so every widget it renders becomes non-interactive.
				ImGui::BeginDisabled(true);
				const_cast<Component*>(comp)->DrawInInspector();
				ImGui::EndDisabled();
				ImGui::Separator();
			}
		}

		if (compOrder.empty())
			ImGui::TextDisabled("No components attached.");

		ImGui::End();
	}

	void BuildStatusBar(GameObject* hovered)
	{
		const ImGuiViewport* vp = ImGui::GetMainViewport();
		ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, 24.f));
		ImGui::SetNextWindowBgAlpha(0.7f);

		ImGuiWindowFlags flags =
			ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings;

		ImGui::Begin("##debugger_status", nullptr, flags);

		if (hovered)
		{
			const Transform& wt = hovered->worldTransform();
			ImGui::Text("Hovered: %s  |  world pos (%.2f, %.2f)  |  components: %zu",
				hovered->cname().c_str(),
				wt.position.x, wt.position.y,
				hovered->componentOrder().size());
		}
		else
		{
			ImGui::TextDisabled("Move mouse over a GameObject to inspect.");
		}

		ImGui::End();
	}
}

namespace Debugger{
	void DrawUI(DebuggerScene& dscene, Scene& scene, GameObject* hovered)
	{
		BuildDockSpcae();
		BuildMenuBar(dscene);
		BuildHierarchyWindow(scene, hovered);
		BuildInspectorWindow(hovered);
		BuildStatusBar(hovered);
	}
}

void DebuggerScene::OnEnter()
{
	SceneIO::DeserializeScene(_loadedScene, "physics_test");
	ParticleSystem::Initialize();
	CameraSystem::OnStart();
}

void DebuggerScene::OnUpdate()
{
	CameraSystem::OnUpdate();
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

	_hoveredObject = Debugger::PickHoveredObject(_loadedScene);

	for (auto& pgo : _loadedScene.gameObjectList())
	{
		pgo->UpdateWorldTransform();
		UpdateGO(*pgo);
	}

	ParticleSystem::Update();
	Graphics::Execute();
	ParticleSystem::Render();
	
	if (_hoveredObject)
		DrawHoverOutline(*_hoveredObject);

	if (EngineCTX::imguiInitialize)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		Debugger::DrawUI(*this, _loadedScene, _hoveredObject);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGui::EndFrame();
	}
}

void DebuggerScene::OnExit()
{
	Graphics::Flush();
	Physics::Flush();
	CameraSystem::OnExit();
}