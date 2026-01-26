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

namespace {
	std::vector<Collider*> _colliders;
	std::unordered_set<Collider*> _colliderSet;

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
		float dx = tb.position.x - ta.position.x;
		float dy = tb.position.y - ta.position.y;

		float distance_sq = dx * dx + dy * dy;

		float rad_sum = (a.radius * ta.scale.x) + (b.radius * tb.scale.x);

		return distance_sq < (rad_sum * rad_sum);
	}

	bool BoxVSBox(const BoxCollider& a, const BoxCollider& b, const Transform& ta, const Transform& tb)
	{
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
				return false;
			}
		}

		return true;
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

				if (c1->name() == "BoxCollider")
				{
					auto* b1 = dynamic_cast<BoxCollider*>(c1);
					
					if (c2->name() == "BoxCollider")
					{
						auto* b2 = dynamic_cast<BoxCollider*>(c2);
						if (BoxVSBox(*b1, *b2, *t1, *t2))
						{
							std::cout << c1->gameObject().name() << " and " << c2->gameObject().name() << " are colliding! (Box Vs Box)\n";
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
}