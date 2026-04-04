/*
Author: Zachary Yee
Co-Author: Nil
*/
#pragma once

namespace LevelTransition
{
	// Enum states for level transition.
	enum class TransitionState : char
	{
		TRANSITION_NULL,
		TRANSITION_FADEIN,
		TRANSITION_FADEOUT,
		TRANSITION_COUNT
	};

	// Boolean states used in lvl transition.
	inline bool inTransition{ false };
	inline bool restartCalled{ false };

	// Function declarations.
	void Init();
	void RequestTransition(f32 setTime = 1.5f);
	void Update();
}