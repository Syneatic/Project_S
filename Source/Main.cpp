#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_opengl3.h"
#include "ImGUI/imgui_impl_win32.h"

#include <crtdbg.h> // To check for memory leaks
#include <vector>
#include <string>
#include <iostream>
#include <typeindex>

#include "AEEngine.h"

#include "gameobject.hpp"
#include "component.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "scene_editor.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT ImGuiWNDCallBack(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	// Give ImGui first chance to handle input messages.
	// If it returns true, ImGui consumed it and you should not pass it on.
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp))
		return 1;

	return DefWindowProc(hWnd, msg, wp, lp);
}

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
	SceneManager& sceneManager = SceneManager::Instance();
	EditorScene editorScene{};
	Scene blankScene{};
	sceneManager.RequestSceneSwitch(&editorScene);

	// Using custom window procedure
	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, ImGuiWNDCallBack);

	// Changing the window title
	AESysSetWindowTitle("Project S");
	AEInputShowCursor(1);

	// Initialize render system
	renderSys::rendererInit();


	InitializeImGUI(m_ImGUIInitialized);
	editorScene.imguiInitialized = true;

	// Game Loop
	while (gGameRunning)
	{
		// Informing the system about the loop's start
		AESysFrameStart();
		f32 dt = (f32)AEFrameRateControllerGetFrameTime();
		// Your own rendering logic goes here
		// Set the background to black.
		AEGfxSetBackgroundColor(0.f, 0.f, 0.f);


		if (AEInputCheckTriggered(AEVK_1)) sceneManager.RequestSceneSwitch(&editorScene);
		if (AEInputCheckTriggered(AEVK_2)) sceneManager.RequestSceneSwitch(&blankScene);


		sceneManager.OnUpdate();

		// Informing the system about the loop's end
		AESysFrameEnd();

		// check if forcing the application to quit
		if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
		gGameRunning = 0;
	}

	ShutdownImGUI(m_ImGUIInitialized);
	renderSys::rendererExit();
	// free the system
	AESysExit();
}
