#include "camera.hpp"
#include "camera_components.hpp"



int fX, fY, zoom;
float posX{}, posY{}, offX{}, offY{};
bool scrollHeld{};

namespace CameraSystem 
{
	void OnStart() 
	{
		AEMtx33Identity(&CameraData::camMatrix);
		//AEGfxSetCamPosition(0,0);
		offX = 0; offY = 0;
	}

	void OnUpdate() 
	{
		AEMtx33 camPos{};

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
			//AEGfxGetCamPosition(&posX, &posY);
			//AEGfxSetCamPosition(posX - dX/ CameraData::zoomMult, posY + dY/ CameraData::zoomMult);
			AEMtx33Trans(&camPos, posX -= (dX / CameraData::zoomMult), posY += (dY / CameraData::zoomMult));
			AEMtx33Concat(&CameraData::camMatrix, &CameraData::camMatrix, &camPos);

			// reset first X and Y on release
			if (AEInputCheckReleased(AEVK_MBUTTON)){
				fX = 0; fY = 0;
				scrollHeld = false;
			}
		}

		//AEInputMouseWheelDelta(&zoom); // check scroll
		zoom = 0;
		if (AEInputCheckTriggered(AEVK_O)) { zoom = 1; };
		if (AEInputCheckTriggered(AEVK_I)) { zoom = -1; };
		if (zoom != 0) 
		{
			const float step = 1.5f; // 50% per notch
			if (zoom > 0) CameraData::zoomMult *= step;
			else          CameraData::zoomMult /= step;
		}

		// Translate to origin, scale, translate back
		AEMtx33 scale{}, negCamPos{}, tempCamPos{};
		AEMtx33Scale(&scale, CameraData::zoomMult, CameraData::zoomMult);
		AEMtx33Trans(&negCamPos, -posX, -posY);
		AEMtx33Trans(&tempCamPos, posX, posY);

		AEMtx33Concat(&negCamPos, &scale, &negCamPos);
		AEMtx33Concat(&CameraData::camMatrix, &tempCamPos, &negCamPos);
	}

	void OnExit() 
	{
		AEMtx33Identity(&CameraData::camMatrix);
		//AEGfxSetCamPosition(0, 0);
		CameraData::zoomMult = 1;
	}

	void MoveCamera(Transform parentTrans) 
	{
		// fix dis
		AEMtx33 camPos{};
		offX = -parentTrans.position.x; offY = -parentTrans.position.y;
		AEMtx33Trans(&camPos, offX, offY);
		AEMtx33Identity(&CameraData::camMatrix);
		AEMtx33Concat(&CameraData::camMatrix, &camPos, &CameraData::camMatrix);
	}

	float2 ScreenToWorld(float2 pos)
	{
		float2 worldPos;
		float2 screen{(f32)AEGfxGetWindowWidth(),(f32)AEGfxGetWindowHeight()};
		float2 camera{};
		AEGfxGetCamPosition(&camera.x, &camera.y);

		float ndcX = (pos.x / screen.x) - 0.5f;
		float ndcY = 0.5f - (pos.y / screen.y);

		worldPos.x = (ndcX * screen.x / CameraData::zoomMult) + camera.x;
		worldPos.y = (ndcY * screen.y / CameraData::zoomMult) + camera.y;


		return worldPos;
	}
}