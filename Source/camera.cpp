#include "AEEngine.h"
#include "camera.hpp"
#include <iostream>


int fX, fY, zoom;
float posX, posY, zoomMult{ 1 };
bool scrollHeld{};

namespace CameraSystem {

	void OnStart() {
		AEMtx33Identity(&CameraData::camMatrix);
		AEGfxSetCamPosition(0,0);
	}

	void OnUpdate() {
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
			AEGfxSetCamPosition(posX - dX/zoomMult, posY + dY/ zoomMult);

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
			if (zoom > 0) zoomMult *= step;
			else           zoomMult /= step;
		}

		// Translate to origin, scale, translate back
		AEMtx33 scale{}, negCamPos{}, camPos{};
		AEMtx33Scale(&scale, zoomMult, zoomMult);
		AEMtx33Trans(&negCamPos, -posX, -posY);
		AEMtx33Trans(&camPos, posX, posY);

		AEMtx33Concat(&negCamPos, &scale, &negCamPos);
		AEMtx33Concat(&CameraData::camMatrix, &camPos, &negCamPos);
	}

	void OnExit() {
		AEMtx33Identity(&CameraData::camMatrix);
		AEGfxSetCamPosition(0, 0);
		zoomMult = 1;
	}

	void MoveCamera(Transform parentTrans) {

	}
}