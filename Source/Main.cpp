#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_opengl3.h"
#include "ImGUI/imgui_impl_win32.h"

#include <crtdbg.h> // To check for memory leaks
#include <vector>
#include <string>
#include <iostream>
#include <typeindex>
#include "AEEngine.h"

#include "scene.hpp"
#include "scene_parser.hpp"
#include "gameobject.hpp"
#include "component.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT ImGuiWNDCallBack(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	// Give ImGui first chance to handle input messages.
	// If it returns true, ImGui consumed it and you should not pass it on.
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp))
		return 1;

	return DefWindowProc(hWnd, msg, wp, lp);
}

//dummy gameobject list
std::vector<std::unique_ptr<GameObject>> gameObjectList{};

void InitializeImGUI(bool& initStatus)
{
	ImGui_ImplWin32_EnableDpiAwareness();
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//add other flags here

	ImGui::StyleColorsDark();
	
	HWND hwnd = AESysGetWindowHandle();
	ImGui_ImplWin32_InitForOpenGL(hwnd);
	ImGui_ImplOpenGL3_Init("#version 130");

	initStatus = true;
}

void ShutdownImGUI(bool& initStatus)
{
	if (!initStatus) return;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	initStatus = false;
}

//this function will be migrated to the editor scene
void BuildImGUI()
{
	static int selected = -1;
	//build scene window
	ImGui::SetNextWindowSizeConstraints(ImVec2(320.f,100.f),ImVec2(FLT_MAX,FLT_MAX));
	ImGui::Begin("Scene");
	//iterate through scene objects and display them here
	for (size_t i = 0; i < gameObjectList.size(); i++)
	{
		GameObject& gobj = *gameObjectList[i];
		bool isSelected = (selected == i);


		if (ImGui::Selectable(gobj.name().c_str(),isSelected))
		{
			//set selected object index
			selected = (int)i;
		}
	}

	if (ImGui::BeginPopupContextWindow("SceneRightClickMenu"))
	{
		if (ImGui::MenuItem("Create GameObject"))
		{
			int index = (int)gameObjectList.size();
			std::cout << "Create GameObject_" << index << std::endl;
			std::string name = "GameObject_" + std::to_string(index);
			gameObjectList.push_back(std::make_unique<GameObject>(name));
			selected = index;
		}

		if (ImGui::MenuItem("Delete GameObject"))
		{
			gameObjectList.erase(gameObjectList.begin() + selected);
			selected = -1;
		}

		ImGui::EndPopup();
	}

	ImGui::End();

	//build the inspector window
	ImGui::SetNextWindowSizeConstraints(ImVec2(320.f, 100.f), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::Begin("Inspector");

	//check if an object is selected
	if (selected < 0) { ImGui::End(); return; }
	
	//display selected object's properties
	//iterate through each component and display its properties here
	//name text box
	GameObject& selectedObj = *gameObjectList[selected];

	//game object properties
	char nameBuffer[256];
	strcpy_s(nameBuffer, selectedObj.name().c_str());
	if (ImGui::InputText(" ", nameBuffer, sizeof(nameBuffer)))
	{
		//update name if changed
		selectedObj.name(std::string(nameBuffer));
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

	//ImGui::Spacing();
	ImGui::Separator();

	if (ImGui::Button("Add Component"))
	{
		ImGui::OpenPopup("bang");
	}

	if (ImGui::BeginPopup("bang"))
	{
		if (ImGui::MenuItem("Transform"))
		{
			std::cout << "Transform" << std::endl;
			selectedObj.AddComponent(
				Transform()
			);
		}

		if (ImGui::MenuItem("Circle Collider"))
		{
			std::cout << "Circle Collider" << std::endl;
			selectedObj.AddComponent(
				CircleCollider()
			);
		}

		if (ImGui::MenuItem("Box Collider"))
		{
			std::cout << "Circle Collider" << std::endl;
			selectedObj.AddComponent(
				BoxCollider()
			);
		}

		if (ImGui::MenuItem("Renderer"))
		{
			std::cout << "Renderer" << std::endl;
		}

		ImGui::EndPopup();
	}

	ImGui::End();
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	int gGameRunning = 1;

	// Initialization of your own variables go here
	bool m_ImGUIInitialized = false;

	// Using custom window procedure
	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, ImGuiWNDCallBack);

	// Changing the window title
	AESysSetWindowTitle("Project S");
	AEInputShowCursor(1);


	InitializeImGUI(m_ImGUIInitialized);

	//testing
	for (size_t i = 0; i < 10; i++)
	{
		std::string name = "GameObject_" + std::to_string(i);
		gameObjectList.push_back(std::make_unique<GameObject>(name));
		gameObjectList.back()->AddComponent(
			Transform(float2((f32)i, (f32)i),
				float2((f32)i, (f32)i), (f32)i)
		);
	}

	Scene test_scn("TEST_SCENE");
	//SceneIO::SerializeScene(test_scn,"TEST");

	// Game Loop
	while (gGameRunning)
	{
		// Informing the system about the loop's start
		AESysFrameStart();
		f32 dt = (f32)AEFrameRateControllerGetFrameTime();
		// Your own rendering logic goes here
		// Set the background to black.
		AEGfxSetBackgroundColor(0.f, 0.f,0.f);
	
		// ===== IMGUI FRAME =====
		if (m_ImGUIInitialized)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			BuildImGUI();
			//ImGui::ShowDemoWindow();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}
	
		// Informing the system about the loop's end
		AESysFrameEnd();

		// check if forcing the application to quit
		if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
			gGameRunning = 0;
	}

	ShutdownImGUI(m_ImGUIInitialized);
	// free the system
	AESysExit();
}


