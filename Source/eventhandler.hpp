#pragma once

#include <unordered_map>
#include <vector>
#include <functional>
#include <string>
#include <typeindex>
#include <typeinfo>
#include "event.hpp"

namespace EventHandler
{
	using EventCallback = std::function<void(IEvent*)>;
	std::unordered_map<std::type_index, std::vector<EventCallback>> subscribers;
	std::vector<std::unique_ptr<IEvent>> eQueue;

	template<typename T>
	void Subscribe(EventCallback cb)
	{
		subscribers[typeid(T)].push_back(cb);
	}
	
	template <typename T, typename... Args>
	void RaiseEvent(Args&& ...)
	{
		eQueue.push_back(std::make_unique<T>(std::forward<Args>(args)...));
	}
	
	void CallQ();
	void FlushQ();
}

//class Dispatcher
//{
//	Dispatcher() {}
//	
//	using EventCallback = std::function<void(IEvent*)>;
//	std::unordered_map<std::type_index, std::vector<EventCallback>> subscribers;
//	std::vector<std::unique_ptr<IEvent>> eQueue;
//public:
//	// Singleton.
//	static Dispatcher& Instance()
//	{
//		static Dispatcher instance;
//		return instance;
//	}
//
//	Dispatcher(const Dispatcher&) = delete;
//	Dispatcher& operator=(const Dispatcher&) = delete;
//	Dispatcher(Dispatcher&&) = delete;
//	Dispatcher& operator=(Dispatcher&&) = delete;
//
//	template<typename T>
//	void Subscribe(EventCallback cb)
//	{
//		subscribers[typeid(T)].push_back(cb);
//	}
//
//	//void RaiseEvent(std::unique_ptr<IEvent> e);
//	template <typename T, typename... Args>
//	void RaiseEvent(Args&& ...)
//	{
//		m_queue.push_back(std::make_unique<T>(std::forward<Args>(args)...));
//	}
//
//	void FlushQ();
//};