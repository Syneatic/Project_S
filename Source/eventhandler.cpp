#include "eventhandler.hpp"

//void Dispatcher::RaiseEvent(std::unique_ptr<IEvent> e)
//{
//	eQueue.push_back(std::move(e));
//}

void EventHandler::CallQ()
{
	// Make a temp queue in case events push new events to original queue
	// to prevent possible iteration to out of bounds.
	std::vector<std::unique_ptr<IEvent>> processingQueue;
	processingQueue.swap(eQueue);

	for (auto& event : processingQueue)
	{
		auto type{ event->getType() };

		if (subscribers.count(type))
		{
			for (auto& callback : subscribers[type])
			{
				callback(event.get());
			}
		}
	}
}

void EventHandler::FlushQ()
{
	eQueue.clear();
}