#pragma once

namespace LevelTransition
{
	enum class TransitionState : char
	{
		TRANSITION_NULL,
		TRANSITION_FADEIN,
		TRANSITION_FADEOUT,
		TRANSITION_COUNT
	};

	inline bool inTransition{ false };

	void Init();
	void RequestTransition(f32 setTime = 1.5f);
	void Update();
}