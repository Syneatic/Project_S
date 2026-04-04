/*
Author: Zachary Yee
Co-Author: Nil
*/
#include "eventhandler.hpp"

namespace EventHandler
{
	// Execute all function callbacks on queue and pop them at end of frame.
	void CallQ()
	{
		while(eQueue.size())
		{
			auto& event = *(eQueue.front()); // Get reference to dereferenced IEvent at front of queue.
			auto type = event.getType(); // Get the std::type_index of the IEvent.
			auto it = subscribers.find(type); // Find the iterator to event inner map element in subscriber outer map.

			// Check iterator is valid.
			if (it != subscribers.end())
			{
				// Iterate each event element in inner map directly.
				for (auto &kv : it->second) 
				{ 
					// Get reference to function callback in inner map event element.
					auto &callback = kv.second; 
					if (callback) 
						callback(&event); 
				}
			}

			eQueue.pop();
		}
	}

	// Flush subscribers & queue (e.g. on scene switch)
	void Flush()
	{
		std::queue<std::unique_ptr<IEvent>> empty; 
		std::swap(eQueue, empty);
		subscribers.clear();
	}

	// Unsubscribe event if needed.
	void Unsubscribe(const SubscriptionHandle& handle)
	{
		auto it = subscribers.find(handle.type);
		if (it != subscribers.end())
		{
			it->second.erase(handle.id);
			if (it->second.empty())
				subscribers.erase(it);
		}
	}
}
