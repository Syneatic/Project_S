#include <windows.h>
#include <shobjidl.h> 
#include <string>
#include <filesystem>
#include <iostream>

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_opengl3.h"
#include "ImGUI/imgui_impl_win32.h"

//systems
#include "renderer.hpp"
#include "physics.hpp"
#include "gameobject.hpp"
#include "audio.hpp"

//scene
#include "scene_parser.hpp"
#include "scene_editor.hpp"

//comps
#include "components.hpp"



namespace
{
	std::wstring OpenFile()
	{
		//get current directory
		//wchar_t cwd[MAX_PATH]{};
		//GetCurrentDirectoryW(MAX_PATH, cwd);
		namespace fs = std::filesystem;
		std::wstring targetDir = L"../../Assets/Scene/";

		try {
			if (fs::exists(targetDir)) {
				targetDir = fs::absolute(targetDir).wstring();
			}
		}
		catch (...) {}

		//create dialog
		HRESULT hrInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		bool didCoInit = SUCCEEDED(hrInit) || hrInit == RPC_E_CHANGED_MODE;

		//check if successful
		IFileOpenDialog* pfd = nullptr;
		HRESULT hrCreate = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
		if (FAILED(hrCreate) || !pfd)
		{
			if (didCoInit && hrCreate != RPC_E_CHANGED_MODE) CoUninitialize();
			return L"";
		}

		//configure dialog
		pfd->SetTitle(L"Open Scene");

		// Optional: filter
		COMDLG_FILTERSPEC filters[] =
		{
			{ L"Scene files (*.scene)", L"*.scene" },
			{ L"All files (*.*)",       L"*.*"     }
		};
		pfd->SetFileTypes((UINT)std::size(filters), filters);
		pfd->SetFileTypeIndex(1);

		//starting folder
		IShellItem* startFolder = nullptr;
		if (SUCCEEDED(SHCreateItemFromParsingName(targetDir.c_str(), nullptr, IID_PPV_ARGS(&startFolder))))
		{
			pfd->SetFolder(startFolder);
			pfd->SetDefaultFolder(startFolder);
			startFolder->Release();
		}

		//display dialog
		HRESULT hrShow = pfd->Show(nullptr);

		std::wstring out;
		if (SUCCEEDED(hrShow))
		{
			IShellItem* result = nullptr;
			if (SUCCEEDED(pfd->GetResult(&result)) && result)
			{
				PWSTR path = nullptr;
				if (SUCCEEDED(result->GetDisplayName(SIGDN_FILESYSPATH, &path)) && path)
				{
					out = path;
					CoTaskMemFree(path);
				}
				result->Release();
			}
		}

		pfd->Release();
		if (didCoInit && hrInit != RPC_E_CHANGED_MODE)
			CoUninitialize();

		return out;
	}

	void RegisterSceneRenderers(const Scene& scene)
	{
		for (auto& pgo : scene.gameObjectList())
		{
			auto* go = pgo.get();
			for (auto& [type, comp] : go->componentMap())
				if (auto* r = dynamic_cast<Renderer*>(comp.get()))
					RenderSystem::RegisterRenderer(r);
		}
	}

	void UnregisterSceneRenderers(const Scene& scene)
	{
		for (auto& pgo : scene.gameObjectList())
		{
			auto* go = pgo.get();
			for (auto& [type, comp] : go->componentMap())
				if (auto* r = dynamic_cast<Renderer*>(comp.get()))
					RenderSystem::UnregisterRenderer(r);
		}
	}

	void RegisterSceneColliders(const Scene& scene)
	{
		for (auto& pgo : scene.gameObjectList())
		{
			auto* go = pgo.get();
			for (auto& [type, comp] : go->componentMap())
				if (auto* c = dynamic_cast<Collider*>(comp.get()))
					Physics::RegisterCollider(c);
		}
	}

	void UnregisterSceneColliders(const Scene& scene)
	{
		for (auto& pgo : scene.gameObjectList())
		{
			auto* go = pgo.get();
			for (auto& [type, comp] : go->componentMap())
				if (auto* c = dynamic_cast<Collider*>(comp.get()))
					Physics::UnregisterCollider(c);
		}
	}

	void RegisterSceneRigidBodies(const Scene& scene)
	{
		for (auto& pgo : scene.gameObjectList())
		{
			auto* go = pgo.get();
			for (auto& [type, comp] : go->componentMap())
				if (auto* rb = dynamic_cast<RigidBody*>(comp.get()))
					Physics::RegisterRigidBody(rb);
		}
	}

	void UnregisterSceneRigidBodeies(const Scene& scene)
	{
		for (auto& pgo : scene.gameObjectList())
		{
			auto* go = pgo.get();
			for (auto& [type, comp] : go->componentMap())
				if (auto* rb = dynamic_cast<RigidBody*>(comp.get()))
					Physics::UnregisterRigidBody(rb);
		}
	}
}

void EditorScene::ReadInput()
{
	s32 mX, mY;
	AEInputGetCursorPosition(&mX, &mY);
	mouseWorld.x = (float)mX - (AEGfxGetWindowWidth() * 0.5f);
	mouseWorld.y = (AEGfxGetWindowHeight() * 0.5f) - (float)mY;

	isMouseDown = AEInputCheckCurr(AEVK_LBUTTON);
	isMousePressed = AEInputCheckTriggered(AEVK_LBUTTON);

	if (AEInputCheckTriggered(AEVK_Q)) currentMode = GizmoMode::TRANSLATE;
	if (AEInputCheckTriggered(AEVK_W)) currentMode = GizmoMode::ROTATE;
	if (AEInputCheckTriggered(AEVK_E)) currentMode = GizmoMode::SCALE;
}

void EditorScene::RefreshRenderers()
{
	RenderSystem::FlushRenderers();                 // clear list
	RegisterSceneRenderers(loadedScene); // rebuild from scene data
}

void EditorScene::RefreshColliders()
{
	Physics::FlushColliders();
	RegisterSceneColliders(loadedScene);
}

void EditorScene::RefreshRigidBodies()
{
	Physics::FlushRigidBody();
	RegisterSceneRigidBodies(loadedScene);
}


// ===== IMGUI =====
void EditorScene::BuildDockSpace()
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

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImGui::Begin("DockHost", nullptr, host_flags);
	ImGui::PopStyleVar(2);

	ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();
}

void EditorScene::BuildMenuBar()
{
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Save"))
		{
			SceneIO::SerializeScene(loadedScene);
		}

		if (ImGui::MenuItem("Load"))
		{
			std::wstring fileW = OpenFile();
			if (!fileW.empty())
			{
				std::filesystem::path p(fileW);

				//std::string fileNameNoExt = p.stem().string();
				SceneIO::DeserializeSceneEditor(loadedScene, p.string());
				selectedGameObjectIndex = -1; //reset index selection
				RefreshRenderers();
				RefreshRigidBodies();
			}
		}

		ImGui::Separator();
		if (ImGui::MenuItem("Quit"))
		{
			//quit the application?
			//or return
		}

		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

void EditorScene::BuildSceneHierarchyWindow()
{
	ImGui::SetNextWindowSizeConstraints(ImVec2(320.f, 100.f), ImVec2(FLT_MAX, FLT_MAX));

	ImGui::Begin("Scene");

	char nameBuffer[256];
	strcpy_s(nameBuffer, loadedScene.name().c_str());
	if (ImGui::InputText(" ", nameBuffer, sizeof(nameBuffer)))
	{
		//update name if changed
		loadedScene.name(std::string(nameBuffer));
	}

	//iterate through scene objects and display them here
	for (int i = 0; i < loadedScene.gameObjectList().size(); i++)
	{
		GameObject& gobj = *loadedScene.gameObjectList()[i];
		bool isSelected = (selectedGameObjectIndex == i);

		if (ImGui::Selectable(gobj.name().c_str(), isSelected))
		{
			//set selected object index
			selectedGameObjectIndex = i;
		}
	}

	if (ImGui::BeginPopupContextWindow("SceneRightClickMenu"))
	{
		if (ImGui::MenuItem("Create GameObject"))
		{
			int index = (int)loadedScene.gameObjectList().size();
			std::cout << "Create GameObject_" << index << std::endl;
			std::string name = "GameObject_" + std::to_string(index);
			loadedScene.gameObjectList().push_back(std::make_unique<GameObject>(name));
			selectedGameObjectIndex = index;
			RefreshRenderers();
		}

		if (selectedGameObjectIndex >= 0)
		{
			if (ImGui::MenuItem("Delete GameObject"))
			{
				loadedScene.gameObjectList().erase(loadedScene.gameObjectList().begin() + selectedGameObjectIndex);
				selectedGameObjectIndex = -1;
				RefreshRenderers();
			}
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}

void EditorScene::BuildInspectorWindow()
{
	ImGui::SetNextWindowSizeConstraints(ImVec2(320.f, 100.f), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::Begin("Inspector");

	//check if an object is selected
	if (selectedGameObjectIndex < 0) { ImGui::End(); return; }

	//display selected object's properties
	//iterate through each component and display its properties here
	//name text box
	GameObject& selectedObj = *loadedScene.gameObjectList()[selectedGameObjectIndex];

	char scnNameBuffer[256];
	strcpy_s(scnNameBuffer, selectedObj.name().c_str());
	if (ImGui::InputText(" ", scnNameBuffer, sizeof(scnNameBuffer)))
	{
		//update name if changed
		selectedObj.name(std::string(scnNameBuffer));
	}

	const auto& comps = selectedObj.componentMap();

	for (auto it = comps.begin(); it != comps.end(); ++it)
	{
		const std::type_index& type = it->first;
		const std::unique_ptr<Component>& compPtr = it->second;

		if (!compPtr) continue;

		if (ImGui::CollapsingHeader(compPtr.get()->name().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					//remove component from game object
					selectedObj.RemoveComponent(type);
					RefreshRenderers();
					ImGui::EndPopup();
					break; //exit loop to avoid invalid iterator
				}
				ImGui::EndPopup();
			}
			compPtr.get()->DrawInInspector();
			ImGui::Separator();
		}
	}

	ImGui::Separator();

	if (ImGui::Button("Add Component"))
	{
		ImGui::OpenPopup("AddComponentMenu");
	}

	if (ImGui::BeginPopup("AddComponentMenu"))
	{
		if (ImGui::MenuItem("Transform"))
		{
			selectedObj.AddComponent<Transform>();
		}

		if (ImGui::BeginMenu("Collider"))
		{
			if (ImGui::MenuItem("Box Collider"))
			{
				selectedObj.AddComponent<BoxCollider>();
			}

			if (ImGui::MenuItem("Circle Collider"))
			{

				selectedObj.AddComponent<CircleCollider>();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Renderer"))
		{
			if (ImGui::MenuItem("Sprite Renderer"))
			{
				selectedObj.AddComponent<SpriteRenderer>();
				RefreshRenderers();
			}

			if (ImGui::MenuItem("Mesh Renderer"))
			{
				selectedObj.AddComponent<MeshRenderer>();
				RefreshRenderers();
			}

			if (ImGui::MenuItem("Text Renderer"))
			{
				selectedObj.AddComponent<TextRenderer>();
				RefreshRenderers();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Controller"))
		{
			if (ImGui::MenuItem("PlayerController"))
			{
				selectedObj.AddComponent<PlayerController>();
			}

			if (ImGui::MenuItem("RockController"))
			{
				selectedObj.AddComponent<RockController>();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Physics"))
		{
			if (ImGui::MenuItem("Rigid Body"))
			{
				selectedObj.AddComponent<RigidBody>();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Particle"))
		{
			if (ImGui::MenuItem("Particle Emitter"))
			{
				selectedObj.AddComponent<ParticleEmitter>();
			}

			if (ImGui::MenuItem("Particle Emitter2"))
			{
				selectedObj.AddComponent<ParticleEmitter2>();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Audio"))
		{
			if (ImGui::MenuItem("Audio Emitter"))
			{
				selectedObj.AddComponent<AudioEmitter>();
			}

			if (ImGui::MenuItem("Audio Listener"))
			{
				selectedObj.AddComponent<AudioListener>();
			}

			ImGui::EndMenu();
		}


		if (ImGui::BeginMenu("UI Type"))
		{
			for (int i = 0; i < IM_ARRAYSIZE(_uiTypes); i++)
			{
				if (ImGui::MenuItem(_uiTypes[i]))
				{
					switch (i)
					{
					case 0:
						selectedObj.AddComponent<Display>();
						break;
						/*case 1:
							selectedObj.AddComponent<Text>();
							break;*/
					case 1:
						selectedObj.AddComponent<Button>();
						break;
					default:
						break;
					}
				}
			}

			ImGui::EndMenu();
		}

		ImGui::EndPopup();
	}

	ImGui::End();
}

void EditorScene::DrawUI()
{
	BuildDockSpace();
	BuildMenuBar();
	BuildSceneHierarchyWindow();
	BuildInspectorWindow();
}
// ===== IMGUI =====

void EditorScene::Gizmos() {
	if (selectedGameObjectIndex < 0) return;

	GameObject& selectedObj = *loadedScene.gameObjectList()[selectedGameObjectIndex];
	Transform* trans = selectedObj.GetComponent<Transform>();
	if (!trans) return;

	//draw selection outline
	RenderSystem::DrawBox(trans->position, trans->scale, trans->rotation, Color(0xFFFC673A));

	// 1. Draw the active gizmo
	switch (currentMode) {
	case GizmoMode::TRANSLATE: DrawTranslationGizmo(trans->position); break;
	case GizmoMode::ROTATE:    DrawRotationGizmo(trans->position); break;
	case GizmoMode::SCALE:     DrawScaleGizmo(trans->position); break;
	}

	// 2. Handle Interaction
	if (isMousePressed) {
		activeAxis = GetHitAxis(mouseWorld, trans->position);

		if (currentMode == GizmoMode::ROTATE && activeAxis == GizmoAxis::ROTATION) {
			startMouseAngle = atan2f(mouseWorld.y - trans->position.y, mouseWorld.x - trans->position.x);
			startObjectRotation = trans->rotation;
		}
		else if (currentMode == GizmoMode::SCALE) {
			startMousePos = mouseWorld;
			startObjectScale = trans->scale;
		}
		else {
			dragOffset = trans->position - mouseWorld;
		}
	}

	if (isMouseDown && activeAxis != GizmoAxis::NONE) {
		if (currentMode == GizmoMode::TRANSLATE) {
			if (activeAxis == GizmoAxis::X || activeAxis == GizmoAxis::CENTER)
				trans->position.x = mouseWorld.x + dragOffset.x;
			if (activeAxis == GizmoAxis::Y || activeAxis == GizmoAxis::CENTER)
				trans->position.y = mouseWorld.y + dragOffset.y;
		}
		else if (currentMode == GizmoMode::ROTATE) {
			float currentAngle = atan2f(mouseWorld.y - trans->position.y, mouseWorld.x - trans->position.x);
			trans->rotation = startObjectRotation + (currentAngle - startMouseAngle) * (180.0f / 3.14159f);
		}
		else if (currentMode == GizmoMode::SCALE) {
			float2 delta = mouseWorld - startMousePos;
			if (activeAxis == GizmoAxis::X) trans->scale.x = startObjectScale.x + delta.x;
			if (activeAxis == GizmoAxis::Y) trans->scale.y = startObjectScale.y + delta.y;
			if (activeAxis == GizmoAxis::CENTER)
			{
				float factor = 1.0f + (delta.x / 100.0f);
				trans->scale = startObjectScale * factor;
			}
		}
	}
	else if (!isMouseDown) {
		activeAxis = GizmoAxis::NONE;
	}
}

void EditorScene::OnEnter()
{
	RefreshRenderers();
	RefreshColliders();
	RefreshRigidBodies();
}

void EditorScene::OnUpdate()
{
	//laze so im refreshing every frame
	RefreshColliders();
	RefreshRigidBodies();

	ReadInput();

	AEGfxSetBackgroundColor(0.f, 0.f, 0.f);
	f32 dt = (f32)AEFrameRateControllerGetFrameTime();

	RenderSystem::Draw();

	bool imguiFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);

	//draw gizmos last
	Gizmos();

	//draw imgui after game render
	if (imguiInitialized)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		DrawUI();
		//ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGui::EndFrame();
	}

}

void EditorScene::OnExit() 
{
	//unload everything
	RenderSystem::FlushRenderers();
	Physics::FlushColliders();
	Physics::FlushRigidBody();
}
