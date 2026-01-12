#include <crtdbg.h> // To check for memory leaks
#include "AEEngine.h"
#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_opengl3.h"
#include "ImGUI/imgui_impl_win32.h"

LRESULT AEEngine_DefaultWinProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp);
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT MyWinCallBack(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	// Give ImGui first chance to handle input messages.
	// If it returns true, ImGui consumed it and you should not pass it on.
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp))
		return 1;

#if USE_AEENGINE_DEFAULT_WNDPROC
	// Let AEEngine handle the rest (recommended if AEEngine exposes this).
	return AEEngine_DefaultWinProc(hWnd, msg, wp, lp);
#else
	// Fallback: standard Win32 handling.
	// Use this only if AEEngine does not require its own message processing.
	return DefWindowProc(hWnd, msg, wp, lp);
#endif
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

void BuildImGUI()
{
	ImGui::Begin("Inspector");
	ImGui::Text("Bang");

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
	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, MyWinCallBack);

	// Changing the window title
	AESysSetWindowTitle("Project S");
	AEInputShowCursor(1);


	InitializeImGUI(m_ImGUIInitialized);

	// Game Loop
	while (gGameRunning)
	{
		// Informing the system about the loop's start
		AESysFrameStart();

		// Your own rendering logic goes here
		// Set the background to black.
		AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);





		
		// ===== IMGUI FRAME =====
		if (m_ImGUIInitialized)
		{
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			//BuildImGUI();
			ImGui::ShowDemoWindow();

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


