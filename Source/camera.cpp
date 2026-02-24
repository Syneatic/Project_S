#include "AEEngine.h"
#include "camera.hpp"


int zoom, fX, fY;
float posX, posY;
bool scrollHeld{};

namespace CameraSystem {

	void OnStart() {
		AEMtx33Identity(&CameraData::camMatrix);
	}

	void OnUpdate() {
		// check middle mouse held down, get first cursor pos
		// and current cursor pos
		if (AEInputCheckTriggered(AEVK_MBUTTON)) {
			AEInputGetCursorPosition(&fX, &fY);
			scrollHeld = true;
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
			AEGfxSetCamPosition(posX - dX, posY + dY);

			// reset first X and Y on release
			if (AEInputCheckReleased(AEVK_MBUTTON)){
				fX = 0; fY = 0;
				scrollHeld = false;
			}
		}

		AEInputMouseWheelDelta(&zoom); // check scroll

		AEMtx33 scale{}, translate{};
		AEMtx33Scale(&scale, zoom, zoom);

		AEMtx33Identity(&CameraData::camMatrix); // reset
		AEMtx33Concat(&CameraData::camMatrix, &scale, &translate);
	}

	void OnExit() {
		AEMtx33Identity(&CameraData::camMatrix);
	}

	void MoveCamera(Transform parentTrans) {

	}
}