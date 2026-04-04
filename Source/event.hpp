/*
Author: Zachary Yee
Co-Author: Yan Chun, Wei Jun
*/
#pragma once

#include "math.hpp"
#include "ui_components.hpp"
#include "level_transition.hpp"

//fwd decl
class GameObject;

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
// UISliderEvent struct
struct UISliderEvent : public Event<UISliderEvent>
{
	AudioSpecifier aS;
	float value{};
	UISliderEvent(AudioSpecifier aS, float value) : aS{ aS }, value{ value } {}
};

// ping event
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

	OnCollisionEvent(GameObject* s, GameObject* o,float2 c, float2 n)
		: self(s), other(o),contactPoint(c), normal(n) { }
};

//physics event
struct OnTriggerEvent : public Event<OnTriggerEvent>
{
	GameObject* self;
	GameObject* other;

	OnTriggerEvent(GameObject* s, GameObject* o)
		: self(s), other(o) {
	}
};