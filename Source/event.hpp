#pragma once

#include <typeindex>
#include "base_components.hpp"
#include "ui_components.hpp"

// Event interface
struct IEvent
{
	virtual ~IEvent() = default;
	virtual std::type_index getType() const = 0;
};

// Base event that uses Curiously Recurring Template Pattern
template <typename Derived>
struct Event : public IEvent 
{
	std::type_index getType() const override { return typeid(Derived); }
};

// UIButtonEvent struct
struct UIButtonEvent : public Event<UIButtonEvent> 
{
	FunctionKey fKey;
	UIButtonEvent(FunctionKey fKey) : fKey{ fKey } {}
};
