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
	};

	inline std::unordered_map<std::type_index, std::unordered_map<CallbackId, EventCallback>> subscribers;
	inline std::queue<std::unique_ptr<IEvent>> eQueue;
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

	void Unsubscribe(const SubscriptionHandle& handle);
	void CallQ();
	void Flush();
}