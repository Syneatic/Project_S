#pragma once

#include "physics_types.hpp"

//forward declare
class Collider;
class BoxCollider;
class RigidBody;

namespace Physics
{
	inline float gravity = 9.81f * 100.f;

	//needs to be registered at start of scene
	void RegisterCollider(Collider* c);
	void RegisterRigidBody(RigidBody* rb);
	void Flush();

	//CALL THIS AFTER REGISTERING ALL COLLIDERS!
	void Initialize();

	void Step();

	bool Raycast(float2 origin, float2 direction, f32 maxDistance, RaycastHit& outHit,u32 layerMask);

	void SyncToLocal();
}