#pragma once
#include <windows.h>
#include <shobjidl.h> 
#include <string>

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_opengl3.h"
#include "ImGUI/imgui_impl_win32.h"

#include "AEEngine.h"
#include "gameobject.hpp"
#include "scene.hpp"
#include "scene_parser.hpp"

static std::wstring OpenFile()
{
	//get current directory
	wchar_t cwd[MAX_PATH]{};
	GetCurrentDirectoryW(MAX_PATH, cwd);

	//create dialog
	HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	bool didCoInit = SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE;

	//check if successful
	IFileOpenDialog* pfd = nullptr;
	hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (FAILED(hr) || !pfd)
	{
		if (didCoInit && hr != RPC_E_CHANGED_MODE) CoUninitialize();
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
	if (SUCCEEDED(SHCreateItemFromParsingName(cwd, nullptr, IID_PPV_ARGS(&startFolder))))
	{
		pfd->SetDefaultFolder(startFolder);
		pfd->SetFolder(startFolder);
		startFolder->Release();
	}

	//display dialog
	HWND owner = AESysGetWindowHandle();
	hr = pfd->Show(owner);
	if (FAILED(hr))
	{
		pfd->Release();
		if (didCoInit && hr != RPC_E_CHANGED_MODE) CoUninitialize();
		return L""; // cancelled or failed
	}

	IShellItem* result = nullptr;
	hr = pfd->GetResult(&result);
	if (FAILED(hr) || !result)
	{
		pfd->Release();
		if (didCoInit && hr != RPC_E_CHANGED_MODE) CoUninitialize();
		return L"";
	}

	PWSTR path = nullptr;
	hr = result->GetDisplayName(SIGDN_FILESYSPATH, &path);

	std::wstring out;
	if (SUCCEEDED(hr) && path)
	{
		out = path;
		CoTaskMemFree(path);
	}

	result->Release();
	pfd->Release();

	if (didCoInit && hr != RPC_E_CHANGED_MODE) CoUninitialize();
	return out;
}

static void RegisterSceneRenderers(const Scene& scene)
{
	auto& rs = RenderSystem::Instance();

	for (auto& pgo : scene.gameObjectList())
	{
		auto* go = pgo.get();
		for (auto& [type, comp] : go->componentMap())
			if (auto* r = dynamic_cast<Renderer*>(comp.get()))
				rs.RegisterRenderer(r);
	}
}

static void UnregisterSceneRenderers(const Scene& scene)
{
	auto& rs = RenderSystem::Instance();

	for (auto& pgo : scene.gameObjectList())
	{
		auto* go = pgo.get();
		for (auto& [type, comp] : go->componentMap())
			if (auto* r = dynamic_cast<Renderer*>(comp.get()))
				rs.UnregisterRenderer(r);
	}
}

struct EditorScene : Scene
{
private:
	Scene loadedScene{}; //current loaded scene data

	// ===== GAMEOBJECT =====
	int selectedGameObjectIndex = -1; // -1 for no selection

	void RefreshRenderers()
	{
		auto& rs = RenderSystem::Instance();
		rs.FlushRenderers();                 // clear list
		RegisterSceneRenderers(loadedScene); // rebuild from scene data
	}

	void BuildDockSpace()
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

	void BuildMenuBar()
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
					selectedGameObjectIndex = -1; //reset index selection
					RefreshRenderers();
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

	void BuildSceneHierarchyWindow()
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

	void BuildInspectorWindow()
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

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Controller"))
			{
				if (ImGui::MenuItem("PlayerController"))
				{
					selectedObj.AddComponent<PlayerController>();
				}

				ImGui::EndMenu();
			}

			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void DrawUI()
	{
		BuildDockSpace();
		BuildMenuBar();
		BuildSceneHierarchyWindow();
		BuildInspectorWindow();
	}

public:
	//temporary boolean here...
	bool imguiInitialized = false;

	void OnEnter() override
	{

	}

	void OnUpdate() override
	{
		//check input here?


		//draw world
		RenderSystem::Instance().Draw();
		//draw gizmos

		//draw imgui after game render
		if (imguiInitialized)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			DrawUI();
			ImGui::ShowDemoWindow();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			ImGui::EndFrame();
		}
	}

	void OnExit() override
	{
		//unload everything
		RenderSystem::Instance().FlushRenderers();
	}

	EditorScene() { _name = "Editor"; }
};