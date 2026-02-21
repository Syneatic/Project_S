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

	template<typename T>
	void Subscribe(EventCallback cb)
	{
		subscribers[typeid(T)].push_back(cb);
	}
	
	template <typename T, typename... Args>
	void RaiseEvent(Args&& ... args)
	{		
		eQueue.push(std::make_unique<T>(std::forward<Args>(args)...));
	}
	
	void CallQ();
	void Flush();
}