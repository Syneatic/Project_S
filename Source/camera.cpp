#include "camera.hpp"
#include "camera_components.hpp"



int fX, fY, zoom;
float posX, posY;
bool scrollHeld{};

namespace CameraSystem 
{
	void OnStart() 
	{
		AEMtx33Identity(&CameraData::camMatrix);
		AEGfxSetCamPosition(0,0);
	}

	void OnUpdate() 
	{
		// check middle mouse held down, get first cursor pos
		// and current cursor pos
		if (AEInputCheckTriggered(AEVK_MBUTTON)) {
			AEInputGetCursorPosition(&fX, &fY);
			scrollHeld = true;
			std::cout << "AABB" << std::endl;
		}
		if (scrollHeld == true) {
			int lX, lY, dX, dY;
			AEInputGetCursorPosition(&lX, &lY);
			// get the difference between current cursor pos
			// and first cursor pos
			dX = lX - fX; dY = lY - fY;
			
			// update first X and Y
			fX = lX; fY = lY;

			// set cam position
			AEGfxGetCamPosition(&posX, &posY);
			AEGfxSetCamPosition(posX - dX/ CameraData::zoomMult, posY + dY/ CameraData::zoomMult);

			// reset first X and Y on release
			if (AEInputCheckReleased(AEVK_MBUTTON)){
				fX = 0; fY = 0;
				scrollHeld = false;
			}
		}

		//AEInputMouseWheelDelta(&zoom); // check scroll
		zoom = 0;
		if (AEInputCheckTriggered(AEVK_I)) { zoom = 1; };
		if (AEInputCheckTriggered(AEVK_O)) { zoom = -1; };
		if (zoom != 0) {
			std::cout << "Works" << std::endl;
			const float step = 1.5f; // 10% per notch
			if (zoom > 0) CameraData::zoomMult *= step;
			else           CameraData::zoomMult /= step;
		}

		// Translate to origin, scale, translate back
		AEMtx33 scale{}, negCamPos{}, camPos{};
		AEMtx33Scale(&scale, CameraData::zoomMult, CameraData::zoomMult);
		AEMtx33Trans(&negCamPos, -posX, -posY);
		AEMtx33Trans(&camPos, posX, posY);

		AEMtx33Concat(&negCamPos, &scale, &negCamPos);
		AEMtx33Concat(&CameraData::camMatrix, &camPos, &negCamPos);
	}

	void OnExit() 
	{
		AEMtx33Identity(&CameraData::camMatrix);
		AEGfxSetCamPosition(0, 0);
		CameraData::zoomMult = 1;
	}

	void MoveCamera(Transform parentTrans) 
	{
		AEMtx33Identity(&CameraData::camMatrix);
		AEGfxSetCamPosition(parentTrans.position.x, parentTrans.position.y);
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