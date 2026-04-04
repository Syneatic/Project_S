/*
Author: Zachary Yee
Co-Author: Yan Chun
*/
#pragma once

#include "event.hpp"

namespace EventHandler
{
	using EventCallback = std::function<void(IEvent*)>;
	using CallbackId = size_t;
	struct SubscriptionHandle 
	{ 
		std::type_index type; 
		CallbackId id;
	};// Object used when unsubscribe is called from other code.

	// Actual nested container for function pointers that becomes listeners.
	inline std::unordered_map<std::type_index, std::unordered_map<CallbackId, EventCallback>> subscribers;
	// Event queue that will be executed at end of frame.
	inline std::queue<std::unique_ptr<IEvent>> eQueue;
	// Unique id for each event listener.
	inline CallbackId nextId{ 0 };


	// Add new subscriber to event handler.
	template<typename T>
	SubscriptionHandle Subscribe(std::function<void(const T&)> cb)
	{
		static_assert(std::is_base_of<IEvent, T>::value, "T must derive from IEvent");

		CallbackId id = nextId++;
		// We wrap the type-specific callback inside the generic IEvent* callback
		subscribers[typeid(T)][id] = [cb](IEvent* e) {
			cb(*static_cast<T*>(e));
			};
		return { typeid(T), id };
	}

	// Push a event to queue.
	template <typename T, typename... Args>
	void RaiseEvent(Args&& ... args)
	{
		eQueue.push(std::make_unique<T>(std::forward<Args>(args)...));
	}
	
	// Cleaned up SubscribeFilter for the new system
	template <typename T, typename M>
	SubscriptionHandle SubscribeFilter(M T::* member, M matchValue, std::function<void(const T&)> func)
	{
		return Subscribe<T>([member, matchValue, func](const T& e)
			{
				if (e.*member == matchValue)
					func(e);
			});
	}

	// Function declarations.
	void Unsubscribe(const SubscriptionHandle& handle);
	void CallQ();
	void Flush();
}