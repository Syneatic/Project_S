#pragma once

//fwd decl
class Collider;

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

struct OBB
{
	float2 center{};
	float2 halfExtents{};  // half width/height in local space
	float2 axisX{};        // local right
	float2 axisY{};
};

struct ContactManifold
{
	Collider* c1 = nullptr;
	Collider* c2 = nullptr;
	float2    normal = {};   // points from c2 -> c1 (push c1 out)
	f32       penetration = 0.f;
	float2    contactPoints[2] = {};
	u32       contactPointCount = 0;
};