#include "gameobject.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "scene_editor.hpp"
#include "scene_parser.hpp"
#include "scene_manager.hpp"
#include "ui_components.hpp"

static WNDPROC g_AEWndProc = nullptr;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT ImGuiWNDCallBack(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	// Give ImGui first chance to handle input messages.
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp))
		return 1;

	// Forward everything else to AE's original WndProc
	if (g_AEWndProc)
		return CallWindowProc(g_AEWndProc, hWnd, msg, wp, lp);

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

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 4.0f;
	style.FrameRounding = 3.0f;
	style.ChildRounding = 3.0f;
	style.ScrollbarRounding = 3.0f;
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
	
	HWND hwnd = AESysGetWindowHandle();
	g_AEWndProc = reinterpret_cast<WNDPROC>(
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ImGuiWNDCallBack))
		);

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

void UpdateEngineCTX()
{
	EngineCTX::unscaledDt = static_cast<f32>(AEFrameRateControllerGetFrameTime());
	EngineCTX::dt = EngineCTX::timeScale * EngineCTX::unscaledDt;
	EngineCTX::frameCount = AEFrameRateControllerGetFrameCount();
	EngineCTX::frameRate = static_cast<f32>(AEFrameRateControllerGetFrameRate());
	EngineCTX::windowSize.x = static_cast<f32>(AEGfxGetWindowWidth());
	EngineCTX::windowSize.y = static_cast<f32>(AEGfxGetWindowHeight());
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	//initialize the engine
	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, nullptr);
	AESysSetWindowTitle("Project S");
	AEInputShowCursor(1);

	// ===== INITIALIZE SYSTEMS =====
	InitializeImGUI(EngineCTX::imguiInitialize);
	Profiler::Get().SetPaused(!EngineCTX::debugMode);

	//grab all scene
	SceneManager::Initialize();

#ifdef _DEBUG
	SceneManager::SwitchToEditor();
#else
	SceneManager::RequestSceneSwitch("Intro");
#endif

	// Initialize render system
	Graphics::Initialize();

	// ===== END INITIALIZE SYSTEMS =====
	
	// Game Loop
	while (EngineCTX::applicationRunning)
	{
		// Informing the system about the loop's start
		AESysFrameStart();

		//update engine ctx
		UpdateEngineCTX();

#ifdef _DEBUG
		ImGuiIO& io = ImGui::GetIO();
		if (!io.WantCaptureMouse && !ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
		{
			if (AEInputCheckTriggered(AEVK_1)) 
				SceneManager::SwitchToEditor();

			if (AEInputCheckTriggered(AEVK_2)) 
				SceneManager::RequestSceneSwitch("Intro");	

			if (AEInputCheckTriggered(AEVK_3)) 
				SceneManager::RequestSceneSwitch("Play_Level");	

			if (AEInputCheckTriggered(AEVK_4))
				SceneManager::RequestSceneSwitch("physics_test");

			if (AEInputCheckTriggered(AEVK_5))
				SceneManager::RequestSceneSwitch("TempSettings");
		}

		if (AEInputCheckTriggered(AEVK_F5))
		{
			EngineCTX::debugMode = !EngineCTX::debugMode;
			Profiler::Get().SetPaused(!EngineCTX::debugMode);
		}
#endif
	
		SceneManager::OnUpdate();
		//std::cout << AEFrameRateControllerGetFrameRate() << "\n";
		// Informing the system about the loop's end
		AESysFrameEnd();

		// check if forcing the application to quit
		if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
			EngineCTX::applicationRunning = false;
	}

	ShutdownImGUI(EngineCTX::imguiInitialize);
	Graphics::Exit();
	// free the system
	AESysExit();
}
