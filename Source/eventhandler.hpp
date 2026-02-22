#pragma once

#include <unordered_map>
#include <vector>
#include <functional>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <queue>
#include "event.hpp"

namespace EventHandler
{
	using EventCallback = std::function<void(IEvent*)>;
	inline std::unordered_map<std::type_index, std::vector<EventCallback>> subscribers;
	inline std::queue<std::unique_ptr<IEvent>> eQueue;

	// Add new subscriber to event handler.
	template<typename T>
	void Subscribe(EventCallback cb)
	{
		subscribers[typeid(T)].push_back(cb);
	}
	
	// Push a event to queue.
	template <typename T, typename... Args>
	void RaiseEvent(Args&& ... args)
	{		
		eQueue.push(std::make_unique<T>(std::forward<Args>(args)...));
	}
	
	void CallQ();
	void Flush();
}

// Function helper for subscribing UI buttons.
inline void SubscribeFunctionKey(FunctionKey key, std::function<void()> func)
{
	// need to have [key, func] cause external params.
	EventHandler::Subscribe<UIButtonEvent>([key, func](IEvent* e)
		{
			auto uiEv = static_cast<UIButtonEvent*>(e);
			if (uiEv->fKey == key)
				func();
		});
}