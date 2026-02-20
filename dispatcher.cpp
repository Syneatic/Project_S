#include "dispatcher.hpp"

template<typename T>
void Dispatcher::Subscribe(EventCallback cb)
{
	subscribers[typeid(T)].push_back(cb);
}

void Dispatcher::RaiseEvent(std::unique_ptr<Event> e)
{
	eQueue.push_back(std::move(e));
}

void Dispatcher::ExecuteQ()
{
	// Make a temp queue in case events push new events to original queue
	// to prevent possible iteration to out of bounds.
	std::vector<std::unique_ptr<Event>> processingQueue;
	processingQueue.swap(eQueue);

	for (auto& e : processingQueue)
	{
		std::type_index type{ typeid(*e) };

		if (subscribers.count(type))
		{
			for (auto& callback : subscribers[type])
			{
				callback(*e);
			}
		}
	}
}