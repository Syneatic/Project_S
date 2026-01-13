#pragma once
#include "math.hpp"

struct Component
{
	virtual void OnStart()
	{

	}

	virtual void OnUpdate()
	{

	}

	virtual void OnDestroy()
	{

	}
};

struct Transform : Component
{
	float2 position{};
	float2 scale{};
	f32 rotation; //stay as float?
};

struct Collider : Component
{

};