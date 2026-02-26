#pragma once

#include "math.hpp"
#include "ui_components.hpp"

//fwd decl
struct GameObject;

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

struct PingEvent : public Event<PingEvent>
{
	size_t targetId;
	PingEvent(size_t target) : targetId{ target } {}
};

//physics event
struct OnCollisionEvent : public Event<OnCollisionEvent>
{
	GameObject* self;
	GameObject* other;
	float2 normal;
	float2 contactPoint;
	float impulse;

	OnCollisionEvent(GameObject* s, GameObject* o,float2 c, float2 n, float i)
		: self(s), other(o),contactPoint(c), normal(n), impulse(i) { }
};