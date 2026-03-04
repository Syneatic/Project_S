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

	void Init();
	void RequestTransition(f32 setTime);
	void CheckState();
	void UnsubscribeTransitions();
}