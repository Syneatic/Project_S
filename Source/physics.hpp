#pragma once
#include <cstdint>

//forward declare
struct float2;
struct Collider;
struct Transform;
struct CircleCollider;
struct BoxCollider;
struct RigidBody;

namespace Physics
{
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
	};

	//needs to be registered at start of scene
	void RegisterCollider(Collider* c);
	void UnregisterCollider(Collider* c);
	void FlushColliders();
	bool CircleVSCircle(const CircleCollider& a, const CircleCollider& b, const Transform& ta, const Transform& tb);
	CollisionInfo BoxVSBox(const BoxCollider& a, const BoxCollider& b, const Transform& ta, const Transform& tb);
	bool Raycast(const float2& origin, const float2& dir, float maxDist, RaycastHit& out, uint32_t layerMask = 0xFFFFFFFF);
	void RegisterRigidBody(RigidBody* rb);
	void UnregisterRigidBody(RigidBody* rb);
	void FlushRigidBody();
	void CheckAllTypeCollisions();
	void Step(float dt);
	RigidBody* CreateRigidBody();
}