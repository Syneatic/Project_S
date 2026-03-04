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
	GameObject* emitterHolder;

	// Function to subscribe to LevelTransitionEvent.
	void SubscribeTransition(TransitionState state, std::function<void(const LevelTransitionEvent&)> func)
	{
		handlers.push_back(EventHandler::SubscribeFilter(&LevelTransitionEvent::tState, state, func));
	}

	std::string SceneToSwitch()
	{
		return SceneManager::ActiveScene()->cname() == "MainMenu" ? "TestScene" : "MainMenu";
	}

	void Init()
	{
		// Subscribe a function that decrement the transition timer & change state if timer runs out.
		SubscribeTransition(TransitionState::TRANSITION_FADEIN, [](const LevelTransitionEvent&) 
			{
				timerFin -= EngineCTX::dt;
				if (timerFin <= 0.f)
				{
					// const std::string& sceneToSwitch{ SceneToSwitch() };
					//emitterHolder->active(false);
					tState = TransitionState::TRANSITION_FADEOUT;
					// SceneManager::RequestSceneSwitch(sceneToSwitch);
				}
			});

		// Subscribe a function that dissable particle effect & calls for the scene to change.
		SubscribeTransition(TransitionState::TRANSITION_FADEOUT, [](const LevelTransitionEvent&) 
			{
				// timerFout -= EngineCTX::dt;
				const std::string& sceneToSwitch{ SceneToSwitch() };
				//emitterHolder->active(false);
				tState = TransitionState::TRANSITION_NULL;
				SceneManager::RequestSceneSwitch(sceneToSwitch); // Remove after particle warm up is implemented.
			});

		emitterHolder = SceneManager::ActiveScene()->FindGameObjectByName("EmitterHolder");
		if (emitterHolder)
			emitterHolder->active(false);

		// Immediately request transition for intro scene.
		if (SceneManager::ActiveScene()->cname() == "Intro")
			RequestTransition(3.f);
	}

	// Call this function if you want to change scene with effect.
	void RequestTransition(f32 tFin/*, f32 tFout*/)
	{
		if (tState == TransitionState::TRANSITION_NULL)
		{
			tState = TransitionState::TRANSITION_FADEIN;
			timerFin = tFin; /*timerFout = tFout;*/
			emitterHolder->active(true);
		}
	}

	// Only raise event if state is either TRANSITION_ON || TRANSITION_OFF.
	void CheckState()
	{
		if (static_cast<bool>(tState))
			EventHandler::RaiseEvent<LevelTransitionEvent>(tState);
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