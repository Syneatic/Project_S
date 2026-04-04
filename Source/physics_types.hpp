/*
Author: Muhammad Harith Bin Khairudyn
Co-Author: Lim Yan Chun
*/
#pragma once

//fwd decl
class Collider;

// Result of a successful raycast hit against a collider.
struct RaycastHit
{
	Collider* collider{};
	float2 point{};
	float2 normal{};
	float distance{};
	uint32_t layerHit{};
};
// Axis-aligned bounding box defined by min and max world-space corners.
struct AABB
{
	float2 min{};
	float2 max{};
};
// Oriented bounding box defined by a center, half-extents, and local axes.
struct OBB
{
	float2 center{};
	float2 halfExtents{};  // half width/height in local space
	float2 axisX{};        // local right
	float2 axisY{};
};
// Collision contact data produced by narrow-phase detection between two colliders.
struct ContactManifold
{
	Collider* c1 = nullptr;
	Collider* c2 = nullptr;
	float2    normal = {};   // points from c2 -> c1 (push c1 out)
	f32       penetration = 0.f;
	float2    contactPoints[2] = {};
	u32       contactPointCount = 0;
};