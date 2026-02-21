#include "eventhandler.hpp"

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

void EventHandler::Flush()
{
	while (!eQueue.empty())
	{
		eQueue.pop();
	}
	subscribers.clear();
}