#pragma once

//forward declare
struct float2;
struct Transform;
class Collider;
class BoxCollider;
class RigidBody;

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
		float2 contactPoint{ 0,0 };
	};

	//needs to be registered at start of scene
	void RegisterCollider(Collider* c);
	void UnregisterCollider(Collider* c);
	void FlushColliders();
	CollisionInfo BoxVSBox(const BoxCollider& a, const BoxCollider& b, const Transform& ta, const Transform& tb);
	bool Raycast(const float2& origin, const float2& dir, float maxDist, RaycastHit& out, uint32_t layerMask = 0xFFFFFFFF);
	void RegisterRigidBody(RigidBody* rb);
	void UnregisterRigidBody(RigidBody* rb);
	void FlushRigidBody();
	void CheckAllTypeCollisions();
	void Step(float dt);
	/*RigidBody* CreateRigidBody();*/
}