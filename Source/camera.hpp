#pragma once

struct Camera;

struct CameraData // Pass to renderer
{
	inline static AEMtx33 camMatrix;
	inline static float zoomMult{ 1 };
};

namespace CameraSystem {
	void OnStart();
	void OnUpdate();
	void OnExit();
	void MoveCamera(Transform parentTrans);

	float2 ScreenToWorld(float2 screen);
}