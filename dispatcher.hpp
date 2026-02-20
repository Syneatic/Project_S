#pragma once

#include <unordered_map>
#include <vector>
#include <functional>
#include <string>
#include <typeindex>
#include <typeinfo>
#include "event.hpp"

class Dispatcher
{
	Dispatcher() {}
	
	using EventCallback = std::function<void(const Event&)>;
	std::unordered_map<std::type_index, std::vector<EventCallback>> subscribers;
	std::vector<std::unique_ptr<Event>> eQueue;

public:
	// Singleton.
	static Dispatcher& Instance()
	{
		static Dispatcher instance;
		return instance;
	}

	Dispatcher(const Dispatcher&) = delete;
	Dispatcher& operator=(const Dispatcher&) = delete;
	Dispatcher(Dispatcher&&) = delete;
	Dispatcher& operator=(Dispatcher&&) = delete;

	template<typename T>
	void Subscribe(EventCallback cb);
	void RaiseEvent(std::unique_ptr<Event> e);
	void ExecuteQ();
};