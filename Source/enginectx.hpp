#pragma once

#include "math.hpp"

namespace EngineCTX
{
	//global storage of engine context, such as window size, delta time, etc
	inline f32 dt{};
	inline f32 unscaledDt{};
	inline f32 fixedDt{ 1.f / 60.f };
	inline f32 timeScale{ 1.0 };
	inline f32 frameRate{};
	inline u32 frameCount{};
	
	inline float2 windowSize{};

	inline bool isPaused{ false };
	inline bool applicationRunning{ true };
	inline bool imguiInitialize{ false };
	inline bool debugMode{ false };

	void PauseTime();
}