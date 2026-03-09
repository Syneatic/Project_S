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
	void OnUpdate();
	void OnExit();
	void MoveCamera(Transform);

	float2 ScreenToWorld(float2 screen);
}