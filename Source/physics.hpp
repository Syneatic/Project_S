/*
Author: Muhammad Harith Bin Khairudyn
Co-Author: Lim Yan Chun
*/
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
	// Registers a collider with the physics system; must be called before Initialize().
	void RegisterCollider(Collider* c);
	// Registers a RigidBody with the physics system; must be called before Initialize().
	void RegisterRigidBody(RigidBody* rb);
	// Clears all registered colliders, RigidBodies, pairs, manifolds, and trigger state.
	void Flush();

	//CALL THIS AFTER REGISTERING ALL COLLIDERS!
	 // Finalises the spatial grid using registered colliders; call after all registrations.
	void Initialize();

	// Advances the simulation by one fixed timestep: integrate, collide, resolve.
	void Step();

	// Casts a ray and returns the closest non-trigger hit within maxDistance on the given layer mask.
	bool Raycast(float2 origin, float2 direction, f32 maxDistance, RaycastHit& outHit,u32 layerMask);
	// Writes each RigidBody's world transform back to its local transform after physics integration.
	void SyncToLocal();
}