#pragma once

#include "enginectx.hpp"

namespace EngineCTX
{
	void PauseTime()
	{
		isPaused = !isPaused;
		timeScale = isPaused ? 0.0 : 1.0;
	}
}