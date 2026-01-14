#pragma once
#include "AETypes.h"

struct float2
{
	f32 x{};
	f32 y{};


	float2() : x(0.0f), y(0.0f)
	{
	}

	float2(f32 ix,f32 iy) : x(ix), y(iy)
	{
	}
};