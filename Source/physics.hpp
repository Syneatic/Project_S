#pragma once
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <vector>
#include <unordered_set>

#include "collider_components.hpp"
#include "component.hpp"
#include "math.hpp"
#include "scene.hpp"           
#include "gameobject.hpp"
#include "physics_component.hpp"

struct RigidBody;

namespace {
	std::vector<Collider*> _colliders;
	std::unordered_set<Collider*> _colliderSet;
	std::vector<RigidBody*> _rigidbodies;
	std::unordered_set<RigidBody*> _rigidbodySet;

	struct CollisionInfo
	{
		bool collided{ false };
		float2 normal{ 0,0 };
		float penetration{ 0 };
	};

	//helper
	float2 rotatePoint(const float2& point, float angle)
	{
		float rad = angle * (3.14159265359f / 180.0f);
		float c = cosf(rad);
		float s = sinf(rad);

		return { point.x * c - point.y * s, point.x * s + point.y * c };
	}

	void getBoxCorner(const Transform& t, const float2& size, float2 out[4])
	{
		float2 half = { size.x * t.scale.x / 2.0f, size.y * t.scale.y / 2.0f };
		float2 local[4] = {
			 {-half.x, -half.y},
			{ half.x, -half.y},
			{ half.x,  half.y},
			{-half.x,  half.y}
		};

		for (int i = 0; i < 4; i++)
		{
			float2 rotated = rotatePoint(local[i], t.rotation);
			out[i] = { rotated.x + t.position.x, rotated.y + t.position.y };
		}
	}

	float2 edgeNormal(const float2& a, const float2& b)
	{
		float2 edge = { b.x - a.x, b.y - a.y };
		float2 normal = { -edge.y, edge.x };
		float len = sqrtf(normal.x * normal.x + normal.y * normal.y);

		if (len == 0) return { 0,0 };
		normal.x /= len;
		normal.y /= len;

		return normal;
	}
	inline void GetAABB(
		const Transform& t,
		const BoxCollider& b,
		float& left, float& right,
		float& bottom, float& top)
	{
		float hx = (b.size.x * t.scale.x) * 0.5f;
		float hy = (b.size.y * t.scale.y) * 0.5f;

		left = t.position.x - hx;
		right = t.position.x + hx;
		bottom = t.position.y - hy;
		top = t.position.y + hy;
	}
	inline bool RayToCircle(
		const float2& origin, const float2& dirN, float maxDist,
		const Transform& t, const CircleCollider& c,
		float& outT, float2& outNormal)
	{
		float2 center = t.position;
		float r = c.radius * t.scale.x; //watch out for one side scale

		float2 oc = origin - center;

		float b = 2.0f * dot(oc, dirN);
		float cc = dot(oc, oc) - r * r;

		// a = 1 because dirN is normalized
		float disc = b * b - 4.0f * cc;
		if (disc < 0.0f) return false;

		float sqrtDisc = std::sqrt(disc);

		float t0 = (-b - sqrtDisc) * 0.5f;
		float t1 = (-b + sqrtDisc) * 0.5f;

		// choose nearest valid
		float tHit = FLT_MAX;
		if (t0 >= 0.0f) tHit = t0;
		else if (t1 >= 0.0f) tHit = t1;
		else return false;

		if (tHit > maxDist) return false;

		float2 p = origin + (dirN * tHit);
		outNormal = normalize(p - center);
		outT = tHit;
		return true;
	}
	inline void ResolveBoxVsBox(
		RigidBody& rb,
		Transform& t,
		const BoxCollider& a,
		const Transform& ot,
		const BoxCollider& b)
	{
		float lA, rA, bA, tA;
		float lB, rB, bB, tB;

		GetAABB(t, a, lA, rA, bA, tA);
		GetAABB(ot, b, lB, rB, bB, tB);

		float overlapX = (std::min)(rA, rB) - (std::max)(lA, lB);
		float overlapY = (std::min)(tA, tB) - (std::max)(bA, bB);

		if (overlapX <= 0 || overlapY <= 0)
			return;

		// Resolve on smaller axis
		if (overlapY < overlapX)
		{
			// Vertical collision
			if (t.position.y > ot.position.y)
			{
				// Landing
				t.position.y += overlapY;
				rb.velocity.y = 0;
				rb.Is_Grounded = true;
			}
			else
			{
				// Hit from below
				t.position.y -= overlapY;
				rb.velocity.y = 0;
			}
		}
		else
		{
			// Horizontal collision
			if (t.position.x > ot.position.x)
				t.position.x += overlapX;
			else
				t.position.x -= overlapX;

			rb.velocity.x = 0;
		}
	}

	inline bool RayToOBB(
		const float2& origin, const float2& dirN, float maxDist,
		const Transform& t, const BoxCollider& b,
		float& outT, float2& outNormal)
	{
		// half extents in world (box local)
		float hx = (b.size.x * t.scale.x) * 0.5f;
		float hy = (b.size.y * t.scale.y) * 0.5f;
		if (hx < kEps && hy < kEps) return false;

		// Move into box space: rotate by -rotation
		float2 oLocal = rotatePoint(origin - t.position, -t.rotation);
		float2 dLocal = rotatePoint(dirN, -t.rotation);

		float tMin = -FLT_MAX;
		float tMax = FLT_MAX;

		// X slab
		if (absf(dLocal.x) < kEps)
		{
			if (oLocal.x < -hx || oLocal.x > hx) return false;
		}
		else
		{
			float inv = 1.0f / dLocal.x;
			float t1 = (-hx - oLocal.x) * inv;
			float t2 = (hx - oLocal.x) * inv;
			if (t1 > t2) std::swap(t1, t2);
			tMin = (std::max)(tMin, t1);
			tMax = (std::min)(tMax, t2);
			if (tMax < tMin) return false;
		}

		// Y slab
		if (absf(dLocal.y) < kEps)
		{
			if (oLocal.y < -hy || oLocal.y > hy) return false;
		}
		else
		{
			float inv = 1.0f / dLocal.y;
			float t1 = (-hy - oLocal.y) * inv;
			float t2 = (hy - oLocal.y) * inv;
			if (t1 > t2) std::swap(t1, t2);
			tMin = (std::max)(tMin, t1);
			tMax = (std::min)(tMax, t2);
			if (tMax < tMin) return false;
		}

		// If the ray starts inside the box, tMin can be negative; choose the first forward intersection.
		float tHit = (tMin >= 0.0f) ? tMin : tMax;
		if (tHit < 0.0f) return false;
		if (tHit > maxDist) return false;

		// Compute hit point in local to determine face normal
		float2 pLocal = oLocal + (dLocal * tHit);

		// Determine closest face
		float dxPos = absf(pLocal.x - hx);
		float dxNeg = absf(pLocal.x + hx);
		float dyPos = absf(pLocal.y - hy);
		float dyNeg = absf(pLocal.y + hy);

		float2 nLocal = float2::zero();
		float best = dxPos; nLocal = float2(1,0);
		if (dxNeg < best) { best = dxNeg; nLocal = float2(-1, 0); }
		if (dyPos < best) { best = dyPos; nLocal = float2(0, 1); }
		if (dyNeg < best) { nLocal = float2(0, -1); }

		// Rotate normal back to world
		outNormal = normalize(rotatePoint(nLocal, t.rotation));
		outT = tHit;
		return true;
	}
	

}

namespace Physics
{
	struct RaycastHit
	{
		Collider* collider{};
		float2 point{};
		float2 normal{};
		float distance{};
	};

	//needs to be registered at start of scene
	void RegisterCollider(Collider* c)
	{
		if (!c) return;

		if (_colliderSet.insert(c).second)
			_colliders.push_back(c);
	}

	void UnregisterCollider(Collider* c)
	{
		if (!c) return;
		if (_colliderSet.erase(c) == 0) return;

		//find the renderer
		auto it = std::find(_colliders.begin(), _colliders.end(), c);
		if (it != _colliders.end())
		{
			//push to back and pop
			*it = _colliders.back();
			_colliders.pop_back();
		}
	}

	void FlushColliders()
	{
		_colliders.clear();
		_colliderSet.clear();
	}


	bool CircleVSCircle(const CircleCollider& a, const CircleCollider& b, const Transform& ta, const Transform& tb)
	{
		CollisionInfo info;
		float dx = tb.position.x - ta.position.x;
		float dy = tb.position.y - ta.position.y;

		float distance_sq = dx * dx + dy * dy;

		float rad_sum = (a.radius * ta.scale.x) + (b.radius * tb.scale.x);

		return distance_sq < (rad_sum * rad_sum);
	}

	CollisionInfo BoxVSBox(const BoxCollider& a, const BoxCollider& b, const Transform& ta, const Transform& tb)
	{
		CollisionInfo info;
		/*float left_b1 = ta.position.x - (a.size.x * ta.scale.x) / 2.0f;
		float right_b1 = ta.position.x + (a.size.x * ta.scale.x) / 2.0f;
		float top_b1 = ta.position.y + (a.size.y * ta.scale.y) / 2.0f;
		float bot_b1 = ta.position.y - (a.size.y * ta.scale.y) / 2.0f;

		float left_b2 = tb.position.x - (b.size.x * tb.scale.x) / 2.0f;
		float right_b2 = tb.position.x + (b.size.x * tb.scale.x) / 2.0f;
		float top_b2 = tb.position.y + (b.size.y * tb.scale.y) / 2.0f;
		float bot_b2 = tb.position.y - (b.size.y * tb.scale.y) / 2.0f;
		*/

		float2 aCorners[4], bCorners[4];
		getBoxCorner(ta, a.size, aCorners);
		getBoxCorner(tb, b.size, bCorners);

		float2 axes[4] = {
			edgeNormal(aCorners[0], aCorners[1]),
			edgeNormal(aCorners[1], aCorners[2]),
			edgeNormal(bCorners[0], bCorners[1]),
			edgeNormal(bCorners[1], bCorners[2])
		};
		float minPenetration = FLT_MAX;
		float2 collisionNormal = float2::zero();
		for (int i = 0; i < 4; i++)
		{
			float minA = FLT_MAX, maxA = -FLT_MAX;
			float minB = FLT_MAX, maxB = -FLT_MAX;

			for (int j = 0; j < 4; j++)
			{
				float projA = dot(aCorners[j], axes[i]);
				float projB = dot(bCorners[j], axes[i]);

				minA = (std::min)(minA, projA);
				maxA = (std::max)(maxA, projA);

				minB = (std::min)(minB, projB);
				maxB = (std::max)(maxB, projB);
			}

			if (maxA < minB || maxB < minA)
			{
				return info;
			}

			float penetration = (std::min)(maxA - minB, maxB - minA);

			if (penetration < minPenetration)
			{
				minPenetration = penetration;
				collisionNormal = axes[i];

				float2 centerA = ta.position;
				float2 centerB = tb.position;
				float2 dir = centerB - centerA;
				if (dot(dir, collisionNormal) < 0)
				{
					collisionNormal.x = -collisionNormal.x;
					collisionNormal.y = -collisionNormal.y;
				}
			}
		}
		info.collided = true;
		info.normal = collisionNormal;
		info.penetration = minPenetration;
		return info;
	}

	bool Raycast(const float2& origin, const float2& dir, float maxDist, RaycastHit& out)
	{
		float2 dirN = normalize(dir);
		if (lengthsq(dirN) < kEps) return false;
		if (maxDist <= 0.0f) return false;

		bool hitAny = false;
		float bestT = maxDist;

		for (Collider* c : _colliders)
		{
			if (!c) continue;

			auto* tr = c->gameObject().GetComponent<Transform>();
			if (!tr) continue;

			float tHit = 0.0f;
			float2 nHit = float2::zero();
			bool hit = false;

			if (c->name() == "CircleCollider")
			{
				auto* cc = dynamic_cast<CircleCollider*>(c);
				if (cc) hit = RayToCircle(origin, dirN, bestT, *tr, *cc, tHit, nHit);
			}
			else if (c->name() == "BoxCollider")
			{
				auto* bc = dynamic_cast<BoxCollider*>(c);
				if (bc) hit = RayToOBB(origin, dirN, bestT, *tr, *bc, tHit, nHit);
			}

			if (!hit) continue;

			// Keep closest
			if (tHit < bestT)
			{
				bestT = tHit;
				out.collider = c;
				out.distance = tHit;
				out.normal = nHit;
				out.point = origin + (dirN * tHit);
				hitAny = true;
			}
		}

		return hitAny;
	}

	void RegisterRigidBody(RigidBody* rb)
	{
		if (!rb) return;
		if (_rigidbodySet.insert(rb).second)
			_rigidbodies.push_back(rb);
	}

	void UnregisterRigidBody(RigidBody* rb)
	{
		if (!rb) return;
		if (_rigidbodySet.erase(rb) == 0) return;

		auto it = std::find(_rigidbodies.begin(), _rigidbodies.end(), rb);
		if (it != _rigidbodies.end())
		{
			//push to back and pop
			*it = _rigidbodies.back();
			_rigidbodies.pop_back();
		}

	}
	
	void FlushRigidBody()
	{
		_rigidbodies.clear();
		_rigidbodySet.clear();
	}




	void CheckAllTypeCollisions()
	{
		//only iterate through colliders
		for (size_t i = 0; i < _colliders.size(); i++)
		{
			for (size_t j = i + 1; j < _colliders.size(); j++)
			{
				Collider* c1 = _colliders[i];
				Collider* c2 = _colliders[j];
				if (!c1 || !c2 || c1 == c2) continue;

				auto* t1 = c1->gameObject().GetComponent<Transform>();
				auto* t2 = c2->gameObject().GetComponent<Transform>();

				if (!t1 || !t2) continue;

				RigidBody* rb1 = c1->gameObject().GetComponent<RigidBody>();
				RigidBody* rb2 = c2->gameObject().GetComponent<RigidBody>();

				if (c1->name() == "BoxCollider" && c2->name() == "BoxCollider")
				{
					auto* b1 = dynamic_cast<BoxCollider*>(c1);
					auto* b2 = dynamic_cast<BoxCollider*>(c2);

					CollisionInfo info = BoxVSBox(*b1, *b2, *t1, *t2);

					if (info.collided)
					{
						std::cout << c1->gameObject().name() << " and " << c2->gameObject().name() << " are colliding! (Box Vs Box)\n";

						bool rb1Static = (!rb1 || rb1->Is_Static);
						bool rb2Static = (!rb2 || rb2->Is_Static);

						if (rb1Static && rb2Static) continue;

						float2 seperation = info.normal * info.penetration;

						if (!rb1Static && !rb2Static)
						{
							t1->position.x -= seperation.x * 0.5f;
							t1->position.y -= seperation.y * 0.5f;
							t2->position.x += seperation.x * 0.5f;
							t2->position.y += seperation.y * 0.5f;

							// Stop velocity in collision direction
							float vel1 = dot(rb1->velocity, info.normal);
							float vel2 = dot(rb2->velocity, info.normal);
							if (vel1 < 0) rb1->velocity = rb1->velocity - (info.normal * vel1);
							if (vel2 > 0) rb2->velocity = rb2->velocity - (info.normal * vel2);
						}

						else if (!rb1Static)
						{
							// Only obj1 is dynamic (obj2 is static)
							t1->position.x -= seperation.x;
							t1->position.y -= seperation.y;

							// Stop velocity in collision direction
							float vel = dot(rb1->velocity, info.normal);
							if (vel < 0) rb1->velocity = rb1->velocity - (info.normal * vel);

							// Check if grounded (collision normal pointing up from obj1's perspective)
							if (info.normal.y < -0.7f)
							{
								rb1->Is_Grounded = true;
							}
						}

						else // Only obj2 is dynamic (obj1 is static)
						{
							t2->position.x += seperation.x;
							t2->position.y += seperation.y;

							// Stop velocity in collision direction
							float vel = dot(rb2->velocity, info.normal);
							if (vel > 0) rb2->velocity = rb2->velocity - (info.normal * vel);

							// Check if grounded (collision normal pointing up from obj2's perspective)
							if (info.normal.y > 0.7f)
							{
								rb2->Is_Grounded = true;
							}
						}
					}
				}

				if (c1->name() == "CircleCollider")
				{
					auto* b1 = dynamic_cast<CircleCollider*>(c1);

					if (c2->name() == "CircleCollider")
					{
						auto* b2 = dynamic_cast<CircleCollider*>(c2);

						if (CircleVSCircle(*b1, *b2, *t1, *t2))
						{
							std::cout << c1->gameObject().name() << " and " << c2->gameObject().name() << " are colliding! (Circle Vs Circle)\n";
						}
					}
				}
			}
		}
	}

	void Step(float dt)
	{
		std::cout << "=== PHYSICS STEP ===" << std::endl;
		std::cout << "dt: " << dt << std::endl;
		std::cout << "Total rigidbodies: " << _rigidbodies.size() << std::endl;

		// Reset grounded
		for (auto* rb : _rigidbodies)
			if (rb) rb->Is_Grounded = false;

		// Integrate motion
		for (auto* rb : _rigidbodies)
		{
			if (!rb || rb->Is_Static) continue;
			std::cout << rb->name()
				<< " vel=" << rb->velocity.y
				<< " grounded=" << rb->Is_Grounded
				<< " static=" << rb->Is_Static << std::endl;

			Transform* t = rb->gameObject().GetComponent<Transform>();
			if (!t) continue;

			if (rb->Affected_By_Gravity && !rb->Is_Grounded)
				rb->velocity.y -= rb->gravity * dt;

			t->position += rb->velocity * dt;
		}

		// Resolve ALL collisions
		CheckAllTypeCollisions();
	}

	inline RigidBody* CreateRigidBody()
	{
		RigidBody* rb = new RigidBody();
		RegisterRigidBody(rb);
		return rb;
	}
}