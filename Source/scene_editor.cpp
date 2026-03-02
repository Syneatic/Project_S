
//systems
#include "renderer.hpp"
#include "physics.hpp"
#include "gameobject.hpp"
#include "audio.hpp"
#include "camera.hpp"

//scene
#include "scene_parser.hpp"
#include "scene_editor.hpp"

//comps
#include "components.hpp"

//test
#include "imgui_helper.hpp"


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
	selectedGameObjectIndex = -1; //reset index selection
	CameraSystem::OnStart();
	RefreshRenderers();
	RefreshRigidBodies();
}

void EditorScene::RefreshRenderers()
{
	Graphics::Flush();// clear list
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
				std::string fileNameNoExt = p.stem().string();
				SceneIO::DeserializeScene(loadedScene, fileNameNoExt);
				RefreshScene();
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

		ImGui::Selectable(gobj.name().c_str(), isSelected);

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::SetDragDropPayload("GO_REORDER", &i, sizeof(int));
			ImGui::TextUnformatted(gobj.name().c_str()); // tooltip while dragging
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GO_REORDER"))
			{
				int fromIndex = *(const int*)payload->Data;
				if (fromIndex != i)
				{
					// grab the object being dragged
					auto dragged = std::move(loadedScene.gameObjectList()[fromIndex]);
					loadedScene.gameObjectList().erase(loadedScene.gameObjectList().begin() + fromIndex);
					loadedScene.gameObjectList().insert(loadedScene.gameObjectList().begin() + i, std::move(dragged));

					// fix up selected index to follow the moved object
					selectedGameObjectIndex = i;
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked())
			selectedGameObjectIndex = i;
	}

	if (ImGui::BeginPopupContextWindow("SceneRightClickMenu"))
	{
		if (ImGui::MenuItem("Create GameObject"))
		{
			int index = (int)loadedScene.gameObjectList().size();
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
	auto& transform = selectedObj.transform();

	NameInputText(selectedObj.name());


	//draw transform
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::TextUnformatted("Position");
		ImGui::DragFloat2("##transform_position", &transform.position.x, 0.05f);
		ImGui::SameLine();
		if (ImGui::Button("Reset##Pos"))
		{
			transform.position.x = 0.0f;
			transform.position.y = 0.0f;
		}

		ImGui::TextUnformatted("Scale");
		ImGui::DragFloat2("##transform_scale", &transform.scale.x, 0.05f);
		ImGui::SameLine();
		if (ImGui::Button("Reset##Scale"))
		{
			transform.scale.x = 1.0f;
			transform.scale.y = 1.0f;
		}

		ImGui::TextUnformatted("Rotation");
		ImGui::DragFloat("##transform_rotation", &transform.rotation, 0.1f);
		ImGui::SameLine();
		if (ImGui::Button("Reset##Rot"))
		{
			transform.rotation = 0.0f;
		}
		ImGui::Separator();
	}

	const auto& comps = selectedObj.componentMap(); 
	auto& compOrder = selectedObj.componentOrder();
	for (int i = 0; i < (int)compOrder.size(); i++)
	{
		const std::type_index& type = compOrder[i];
		auto it = comps.find(type);
		if (it == comps.end() || !it->second) continue;

		const std::unique_ptr<Component>& compPtr = it->second;

		bool open = ImGui::CollapsingHeader(compPtr->name().c_str(), ImGuiTreeNodeFlags_DefaultOpen);

		// drag source
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
		{
			ImGui::SetDragDropPayload("COMP_REORDER", &i, sizeof(int));
			ImGui::TextUnformatted(compPtr->name().c_str());
			ImGui::EndDragDropSource();
		}

		// drop target
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("COMP_REORDER"))
			{
				int fromIndex = *(const int*)payload->Data;
				if (fromIndex != i)
				{
					auto dragged = compOrder[fromIndex];
					compOrder.erase(compOrder.begin() + fromIndex);
					compOrder.insert(compOrder.begin() + i, dragged);
				}
			}
			ImGui::EndDragDropTarget();
		}

		// right click remove
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Remove Component"))
			{
				selectedObj.RemoveComponent(type);
				ImGui::EndPopup();
				break;
			}
			ImGui::EndPopup();
		}

		if (open)
		{
			compPtr->DrawInInspector();
			ImGui::Separator();
		}
	}

	//for (auto it = comps.begin(); it != comps.end(); ++it)
	//{
	//	const std::type_index& type = it->first;
	//	const std::unique_ptr<Component>& compPtr = it->second;

	//	if (!compPtr) continue;

	//	bool open = ImGui::CollapsingHeader(compPtr.get()->name().c_str(), ImGuiTreeNodeFlags_DefaultOpen);

	//	if (ImGui::BeginPopupContextItem())
	//	{
	//		if (ImGui::MenuItem("Remove Component"))
	//		{
	//			selectedObj.RemoveComponent(type);
	//			RefreshRenderers();
	//			ImGui::EndPopup();
	//			break;
	//		}
	//		ImGui::EndPopup();
	//	}

	//	if (open)
	//	{
	//		compPtr.get()->DrawInInspector();
	//		ImGui::Separator();
	//	}
	//}

	ImGui::Separator();

	if (ImGui::Button("Add Component"))
	{
		ImGui::OpenPopup("AddComponentMenu");
	}

	if (ImGui::BeginPopup("AddComponentMenu"))
	{
		ComponentSubMenu("Physics", { "Box Collider","Circle Collider","Rigid Body" },
			[&](int i)
			{
				if (i == 0) selectedObj.AddComponent<BoxCollider>();
				if (i == 1) selectedObj.AddComponent<CircleCollider>();
				if (i == 2) selectedObj.AddComponent<RigidBody>();
			});

		ComponentSubMenu("Renderer", { "Sprite Renderer","Mesh Renderer" },
			[&](int i)
			{
				if (i == 0) selectedObj.AddComponent<SpriteRenderer>();
				if (i == 1) selectedObj.AddComponent<MeshRenderer>();
			});

		if (ImGui::BeginMenu("Particle"))
		{
			if (ImGui::MenuItem("Particle Emitter"))
			{
				selectedObj.AddComponent<ParticleEmitter>();
			}

			ImGui::EndMenu();
		}

		ComponentSubMenu("Audio", { "Audio Emitter","Audio Listener" },
			[&](int i)
			{
				if (i == 0) selectedObj.AddComponent<AudioEmitter>();
				if (i == 1) selectedObj.AddComponent<AudioListener>();
			});

		ComponentSubMenu("UI", { "Display", "Button", "Text" },
			[&](int i)
			{
				if (i == 0) selectedObj.AddComponent<Display>();
				if (i == 1) selectedObj.AddComponent<Button>();
				if (i == 2) selectedObj.AddComponent<TextRenderer>();
			});


		if (ImGui::MenuItem("Main Camera"))
		{
			selectedObj.AddComponent<MainCamera>();
		}

		ImGui::Separator();
		ImGui::TextUnformatted("Custom");
		ImGui::Separator();
		//we put our own components here
		ComponentSubMenu("Controller", { "Player Controller","Rock Controller","Enemy Controller" },
			[&](int i)
			{
				if (i == 0) selectedObj.AddComponent<PlayerController>();
				if (i == 1) selectedObj.AddComponent<RockController>();
				if (i == 2) selectedObj.AddComponent<EnemyController>();
			});

		if (ImGui::MenuItem("Noise Source"))
		{
			selectedObj.AddComponent<NoiseSource>();
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
	Transform& trans = selectedObj.transform();

	//draw selection outline
	Graphics::RenderData outline;
	outline.alignment = Graphics::Alignment::MC;
	outline.blendMode = Graphics::BlendMode::AE_GFX_BM_NONE;
	outline.drawMode = Graphics::DrawMode::AE_GFX_MDM_LINES;
	outline.color = Color(0xFF'FC'67'3A);
	outline.layer = (Graphics::RenderLayer)(Graphics::RenderLayer::GIZMOS + 25);
	outline.pos = trans.position;
	outline.scale = trans.scale;
	outline.rot = trans.rotation;
	Graphics::Submit(outline,Graphics::PrimitiveType::BOX);

	// 1. Draw the active gizmo
	switch (currentMode) {
	case GizmoMode::TRANSLATE: DrawTranslationGizmo(trans.position); break;
	case GizmoMode::ROTATE:    DrawRotationGizmo(trans.position); break;
	case GizmoMode::SCALE:     DrawScaleGizmo(trans.position); break;
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
	RefreshRenderers();
	RefreshColliders();
	RefreshRigidBodies();
	CameraSystem::OnStart();
}

void EditorScene::OnUpdate()
{
	//laze so im refreshing every frame
	RefreshColliders();
	RefreshRigidBodies();

	ReadInput();
	CameraSystem::OnUpdate(); // Check input and update camera matrix

	AEGfxSetBackgroundColor(0.f, 0.f, 0.f);
	//f32 dt = (f32)AEFrameRateControllerGetFrameTime();
	//bool imguiFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);

	//draw gizmos last
	Gizmos(); //gizmos execution

	//uniquely for editor only
	for (auto& pgo : loadedScene.gameObjectList())
	{
		auto* go = pgo.get();
		for (auto& [type, comp] : go->componentMap())
		{
			if (auto* c = dynamic_cast<Renderer*>(comp.get()))
			{
				c->OnUpdate();
			}
		}
	}

	Graphics::Execute();

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
	Graphics::Flush();
	Physics::FlushColliders();
	Physics::FlushRigidBody();
	CameraSystem::OnExit();
}
