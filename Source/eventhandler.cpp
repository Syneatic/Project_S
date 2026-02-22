#include "eventhandler.hpp"

// Execute all function callbacks on queue and pop them at end of frame.
void EventHandler::CallQ()
{
	while(eQueue.size())
	{
		auto& event = *(eQueue.front());
		auto type = event.getType();

		if (subscribers.count(type))
		{
			for (auto& callback : subscribers[type])
			{
				callback(&event);
			}			
		}
		eQueue.pop();
	}
}

// Flush subscribers & queue (e.g. on scene switch)
void EventHandler::Flush()
{
	while (!eQueue.empty())
	{
		eQueue.pop();
	}
	subscribers.clear();
}