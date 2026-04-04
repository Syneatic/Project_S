#pragma once

#include "enginectx.hpp"

namespace EngineCTX
{
	void PauseTime()
	{
		isPaused = !isPaused;
		timeScale = isPaused ? 0.0f : 1.0f;
	}
}