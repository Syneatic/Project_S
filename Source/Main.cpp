#include "gameobject.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "scene_editor.hpp"
#include "scene_parser.hpp"
#include "scene_manager.hpp"
#include "ui_components.hpp"

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

	//initialize the engine
	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, ImGuiWNDCallBack);
	AESysSetWindowTitle("Project S");
	AEInputShowCursor(1);

	// ===== INITIALIZE SYSTEMS =====

	bool m_ImGUIInitialized = false;
	InitializeImGUI(m_ImGUIInitialized);

	//grab all scene
	SceneManager::Initialize(&gGameRunning);

#ifdef _DEBUG
	SceneManager::SwitchToEditor();
#else
	SceneManager::RequestSceneSwitch("MainMenu");
#endif

	// Initialize render system
	Graphics::Initialize();

	// ===== END INITIALIZE SYSTEMS =====
	
	// Game Loop
	while (gGameRunning)
	{
		// Informing the system about the loop's start
		AESysFrameStart();
		//f32 dt = (f32)AEFrameRateControllerGetFrameTime();

#ifdef _DEBUG
		ImGuiIO& io = ImGui::GetIO();
		if (!io.WantCaptureMouse && !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
		{
			if (AEInputCheckTriggered(AEVK_1)) 
				SceneManager::SwitchToEditor();

			if (AEInputCheckTriggered(AEVK_2)) 
				SceneManager::RequestSceneSwitch("MainMenu");	
		}
#endif

		
		SceneManager::OnUpdate();


		// Informing the system about the loop's end
		AESysFrameEnd();
		////clear results at end of frame
		//Debug::ClearTimerResults();

		//std::cout << AEFrameRateControllerGetFrameRate() << std::endl;
		// check if forcing the application to quit
		if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
		gGameRunning = 0;
	}

	ShutdownImGUI(m_ImGUIInitialized);
	Graphics::Exit();
	// free the system
	AESysExit();
}
