#pragma once

#include "event.hpp"

namespace EventHandler
{
	using EventCallback = std::function<void(IEvent*)>;

	struct SubscriptionHandle 
	{ 
		std::type_index type; 
		size_t id;
	};

	//inline std::unordered_map<std::type_index, std::vector<EventCallback>> subscribers;
	inline std::unordered_map<std::type_index, std::unordered_map<size_t, EventCallback>> subscribers;
	inline std::queue<std::unique_ptr<IEvent>> eQueue;
	inline size_t nextId{ 0 };

	// DEBUG
	//inline int tmp{}, tmp2{};

	// Add new subscriber to event handler.
	template<typename T> 
	SubscriptionHandle Subscribe(EventCallback cb) 
	{ 
		size_t id = nextId++; 
		subscribers[typeid(T)][id] = std::move(cb); 
		return { typeid(T), id }; 
	}

	// Push a event to queue.
	template <typename T, typename... Args>
	void RaiseEvent(Args&& ... args)
	{
		eQueue.push(std::make_unique<T>(std::forward<Args>(args)...));
	}
	
	void Unsubscribe(const SubscriptionHandle& handle);
	void CallQ();
	void Flush();
}

// Helper function to subscribe any event callback to a element in subscribers container.
template <typename T, typename M>
inline EventHandler::SubscriptionHandle SubscribeFilter(M T::* member, M matchValue, std::function<void()> func)
{
	return EventHandler::Subscribe<T>([member, matchValue, func](IEvent* e)
		{
			// Cast the base interface to the specific derived event type
			auto derivedEvent = static_cast<T*>(e);

			// Access the member using the pointer-to-member syntax (->*)
			if (derivedEvent->*member == matchValue)
				func();
		});
}