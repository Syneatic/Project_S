#pragma once

#include "math.hpp"

namespace EngineCTX
{
	//global storage of engine context, such as window size, delta time, etc
	inline f64 dt{};
	inline f64 frameRate{};
	inline u32 frameCount{};

	inline float2 windowSize{};

	inline bool applicationRunning{ true };
	inline bool imguiInitialize{ false };
}