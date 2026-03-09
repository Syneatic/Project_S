#include <vector>
#include "enginectx.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"
#include "eventhandler.hpp"
#include "gameobject.hpp"

namespace LevelTransition
{
	// Container for event subscribers.
	std::vector<EventHandler::SubscriptionHandle> handlers;
	// Global state for level transition.
	TransitionState tState{}; 
	// Float for transition timer.
	f32 timerFin{}, timerFout{};
	// Container for pointers to game objects with particle emitters used for transiton effect.
	GameObject *fadeIn{ nullptr }, *fadeOut{ nullptr };

	bool timerActive = false;

	// Function to subscribe to LevelTransitionEvent.
	void SubscribeTransition(TransitionState state, std::function<void(const LevelTransitionEvent&)> func)
	{
		handlers.push_back(EventHandler::SubscribeFilter(&LevelTransitionEvent::tState, state, func));
	}

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
			tState = TransitionState::TRANSITION_FADEIN;
			timerFin = tFin;
			if (fadeIn) 
				fadeIn->active(true);
			inTransition = !inTransition;
			timerActive = true;
		}
	}


	void Update()
	{
		if (tState == TransitionState::TRANSITION_FADEOUT)
		{
			if (timerFout == 0.f)
			{
				if (fadeIn)
					fadeIn->active(false);
				if (fadeOut)
					fadeOut->active(false);
				const auto* emitter = fadeOut->GetComponent<ParticleEmitter>();
				timerFout = AEGfxGetWindowWidth() / emitter->speed.x;
			}

			timerFout -= EngineCTX::dt;
			if (timerFout <= 0.f)
			{
				inTransition = !inTransition;
				tState = TransitionState::TRANSITION_NULL;
			}
		}

		if (tState == TransitionState::TRANSITION_FADEIN)
		{
			timerFin -= EngineCTX::dt;
			if (timerFin <= 0.f)
			{
				const std::string& sceneToSwitch{ SceneToSwitch() };
				timerFout = 0.f;
				tState = TransitionState::TRANSITION_FADEOUT;
				SceneManager::RequestSceneSwitch(sceneToSwitch);
			}
		}
	}

	// Add unsub for when there is a scene restart here.
	void UnsubscribeTransitions()
	{
		for (auto& sh: handlers)
		{
			EventHandler::Unsubscribe(sh);
		}
		handlers.clear();
	}
}