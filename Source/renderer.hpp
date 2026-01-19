#pragma once
#include "AETypes.h"
#include "math.hpp"

typedef enum drawMode {
	center,
	lSide
}drawMode;

namespace renderSys {
	void rendererInit();

	void drawRect(float2 pos, float rotAngle, float2 size, drawMode alignment);
	void drawTri(float2 pos, float angle, float size);
	void drawCirc(float2 pos, float angle, float size);

	void rendererExit();
}