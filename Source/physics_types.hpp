#pragma once

//fwd decl
struct Collider;

struct RaycastHit
{
	Collider* collider{};
	float2 point{};
	float2 normal{};
	float distance{};
	uint32_t layerHit{};
};

struct CollisionInfo
{
	bool collided{ false };
	float2 normal{ 0,0 };
	float penetration{ 0 };
	float2 contactPoint{ 0,0 };
};

struct AABB
{
	float2 min{};
	float2 max{};
};