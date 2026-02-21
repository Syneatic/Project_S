#pragma once

#include <typeindex>
#include "base_components.hpp"
#include "ui_components.hpp"

struct IEvent
{
	virtual ~IEvent() = default;
	virtual std::type_index getType() const = 0;
};

template <typename Derived>
struct Event : public IEvent 
{
	std::type_index getType() const override { return typeid(Derived); }
};

struct UIButtonEvent : public Event<UIButtonEvent> 
{
	FunctionKey fKey;
	UIButtonEvent(FunctionKey fKey) : fKey{ fKey } {}
};

struct AudioEvent : public Event<AudioEvent>
{

};