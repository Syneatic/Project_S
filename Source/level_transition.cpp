#include <vector>
#include "enginectx.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"
#include "particle.hpp"
#include "eventhandler.hpp"
#include "gameobject.hpp"

namespace LevelTransition
{
	// Global state for level transition.
	TransitionState tState{}; 
	// Float for transition timer.
	f32 timerFin{}, timerFout{};
	// Container for pointers to game objects with particle emitters used for transiton effect.
	GameObject *fadeIn{ nullptr }, *fadeOut{ nullptr };

	// Helper function to determine the scene to load for level transition.
	std::string SceneToSwitch()
	{
		return SceneManager::ActiveScene()->cname() == "MainMenu" ? "TestScene" : "MainMenu";
	}

	// Helper function to get pointer to particle emitter for level transition.
	void FindEmitters()
	{
		fadeIn = SceneManager::ActiveScene()->FindGameObjectByName("FadeIn");
		fadeOut = SceneManager::ActiveScene()->FindGameObjectByName("FadeOut");
	}

	void Init()
	{
		// The first frame of every scene should find emitter game objects.
		FindEmitters();

		// Immediately request transition for intro scene.
		if (SceneManager::ActiveScene()->cname() == "Intro")
			RequestTransition();
	}

	// Call this function if you want to change scene with effect.
	void RequestTransition(f32 tFin)
	{
		if (tState == TransitionState::TRANSITION_NULL)
		{
			if (fadeIn)
				fadeIn->active(true);
			timerFin = tFin;
			inTransition = true;
			EngineCTX::PauseTime();
			tState = TransitionState::TRANSITION_FADEIN;
		}
	}

	// Run this function if tstate in TRANSITION_FADEOUT.
	void FadeOutTimer()
	{
		if (timerFout == 0.f)
		{
			if (fadeIn)
				fadeIn->active(false);

			if (fadeOut)
			{
				fadeOut->active(false);
				auto* emitter = fadeOut->GetComponent<ParticleEmitter>();
				timerFout = AEGfxGetWindowWidth() / emitter->speed.x;
				ParticleSystem::Render();
			}
		}

		timerFout -= EngineCTX::unscaledDt;
		if (timerFout <= 0.f)
		{
			inTransition = false;
			EngineCTX::PauseTime();
			tState = TransitionState::TRANSITION_NULL;
		}
	}

	// Run this function if tstate in TRANSITION_FADEIN.
	void FadeInTimer()
	{
		timerFin -= EngineCTX::unscaledDt;
		if (timerFin <= 0.f)
		{
			timerFout = 0.f;
			tState = TransitionState::TRANSITION_FADEOUT;
			SceneManager::RequestSceneSwitch(SceneToSwitch());
		}
	}

	void Update()
	{
		if (tState == TransitionState::TRANSITION_FADEOUT)
			FadeOutTimer();
		
		if (tState == TransitionState::TRANSITION_FADEIN)
			FadeInTimer();
	}
}