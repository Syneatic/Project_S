#pragma once
#include "AETypes.h"
#include "math.hpp"
#include "component.hpp"

typedef enum drawMode {
	center,
	lSide
}drawMode;

namespace renderSys {
	void rendererInit();

	void drawRect(Transform transform, MeshRenderer renderInfo, drawMode alignment);
	void drawTri(Transform transform, MeshRenderer renderInfo);
	void drawCirc(Transform transform, MeshRenderer renderInfo);

	void rendererExit();
}