#include "camera.hpp"
#include "camera_components.hpp"



int fX, fY, zoom;
bool scrollHeld{};

namespace CameraSystem 
{
	void SetCamM()
	{
		AEMtx33Identity(&CameraData::camM);

		AEMtx33 trans{}, rot{}, scale{};

		AEMtx33Trans(&trans, -CameraData::pos.x, -CameraData::pos.y);
		AEMtx33RotDeg(&rot, -CameraData::rotDeg);
		AEMtx33Scale(&scale, CameraData::zoomMult, CameraData::zoomMult);

		AEMtx33Concat(&CameraData::camM, &trans, &CameraData::camM);
		AEMtx33Concat(&CameraData::camM, &rot, &CameraData::camM);
		AEMtx33Concat(&CameraData::camM, &scale, &CameraData::camM);
	}

	void OnStart() 
	{
		AEMtx33Identity(&CameraData::camM);
		CameraData::zoomMult = 1;
	}

	void OnUpdate() 
	{
		// check middle mouse held down, get first cursor pos
		// and current cursor pos
		if (AEInputCheckTriggered(AEVK_MBUTTON)) 
		{
			AEInputGetCursorPosition(&fX, &fY);
			scrollHeld = true;
		}
		if (scrollHeld == true) 
		{
			int lX, lY, dX, dY;
			AEInputGetCursorPosition(&lX, &lY);
			// get the difference between current cursor pos
			// and first cursor pos
			dX = lX - fX; dY = lY - fY;
			
			// update first X and Y
			fX = lX; fY = lY;

			// set cam position
			CameraData::pos.x -= dX / CameraData::zoomMult;
			CameraData::pos.y += dY / CameraData::zoomMult;

			// reset first X and Y on release
			if (AEInputCheckReleased(AEVK_MBUTTON)){
				fX = 0; fY = 0;
				scrollHeld = false;
			}
		}

		//Debug::Log(AEInputMouseWheelDelta())
		AEInputMouseWheelDelta(&zoom); // check scroll

		if (zoom != 0) 
		{
			const float step = 1.5f; // 50% per notch
			if (zoom > 0) CameraData::zoomMult *= step;
			else          CameraData::zoomMult /= step;
		}

		// Apply scale to camM
		SetCamM();
	}

	void OnExit() 
	{
		AEMtx33Identity(&CameraData::camM);
		CameraData::zoomMult = 1;
	}

	void MoveCamera(Transform playerTrans) 
	{
		// fix dis
		CameraData::pos = playerTrans.position;
		SetCamM();
	}

	float2 ScreenToWorld(float2 pos)
	{
		float2 worldPos;
		float2 screen{(f32)AEGfxGetWindowWidth(),(f32)AEGfxGetWindowHeight()};

		float ndcX = (pos.x / screen.x) - 0.5f;
		float ndcY = 0.5f - (pos.y / screen.y);

		worldPos.x = (ndcX * screen.x / CameraData::zoomMult) + CameraData::pos.x;
		worldPos.y = (ndcY * screen.y / CameraData::zoomMult) + CameraData::pos.y;


		return worldPos;
	}
}