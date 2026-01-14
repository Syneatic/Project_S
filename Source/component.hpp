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

	Transform(float2 pos,float2 scl,f32 rot) : position(pos), scale(scl), rotation(rot)
	{
	}
};

struct Collider : Component
{

};