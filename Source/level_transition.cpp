#include <vector>
#include "enginectx.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"
#include "eventhandler.hpp"

namespace LevelTransition
{
	std::vector<EventHandler::SubscriptionHandle> handlers;
	TransitionState tState{}; f32 timer{2.f};
	std::vector<GameObject*>emitters;

	void SubscribeTransition(TransitionState state, std::function<void(const LevelTransitionEvent&)> func)
	{
		handlers.push_back(EventHandler::SubscribeFilter(&LevelTransitionEvent::tState, state, func));
	}

	void Init()
	{
		SubscribeTransition(TransitionState::TRANSITION_ON, [](const LevelTransitionEvent&) 
			{
				timer -= EngineCTX::dt;
				if (timer <= 0.f)
					tState = TransitionState::TRANSITION_OFF;
			});

		SubscribeTransition(TransitionState::TRANSITION_OFF, [](const LevelTransitionEvent&) 
			{
				const std::string& currScene{SceneManager::ActiveScene()->cname()};
				std::string sceneToSwitch{ currScene == "MainMenu" ? "TestScene" : "MainMenu"};
				//Set emitter inactive here.
				tState = TransitionState::TRANSITION_NULL;
				SceneManager::RequestSceneSwitch(sceneToSwitch);
			});

		if (SceneManager::ActiveScene()->cname() == "Intro")
			RequestTransition(3.f);
	}

	void RequestTransition(f32 setTime)
	{
		if (tState == TransitionState::TRANSITION_NULL)
		{
			tState = TransitionState::TRANSITION_ON;
			timer = setTime;
			// Set particle emitter active here.
		}
	}

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