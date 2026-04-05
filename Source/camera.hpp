/*
Author: Jia Xi
Co-Author: Nil
*/
#pragma once

struct Camera;

struct CameraData // Pass to renderer
{
	inline static AEMtx33 camM = [] {
		AEMtx33 m{};
		AEMtx33Identity(&m);
		return m;
		}();
	inline static float2 pos{ 0.0f, 0.0f };
	inline static float rotDeg{ 0.0f };
	inline static float zoomMult{ 1.0f };
};

namespace CameraSystem {
	void OnStart();
	// Set camera position / zoom in editor
	void OnUpdate();
	// Reset camera position
	void OnExit();

	// Set cam position to player position during playtime
	void MoveCamera(Transform);
	// zoom in/out using scrollwheel
	void ZoomInput();
	// helper function to translate screen position to world position
	float2 ScreenToWorld(float2 screen);
}