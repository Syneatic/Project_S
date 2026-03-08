#pragma once

#include "math.hpp"

namespace EngineCTX
{
	//global storage of engine context, such as window size, delta time, etc
	inline f32 dt{};
	inline f32 fixedDt{ 1.f / 60.f };
	inline f32 frameRate{};
	inline u32 frameCount{};

	inline float2 windowSize{};

	inline bool applicationRunning{ true };
	inline bool imguiInitialize{ false };
}