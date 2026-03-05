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

}