#include "gameobject.hpp"
#include "physics_components.hpp"
#include "physics_types.hpp"
#include "physics.hpp"

//collections
namespace
{
	using BroadPhasePair = std::pair<u32, u32>;
	std::vector<Collider*> _colliders;
	std::vector<RigidBody*> _rigidbodies;
	std::vector<BroadPhasePair> _broadphasePairs;
	std::vector<ContactManifold> _manifolds;

	std::vector<u32> _raycastStamp;
	u32 _currentStamp = 0;
}

//spatial grid helpers
namespace
{
	struct CellCoord
	{
		s32 x, y;
	};

	struct Cell
	{
		//keep the index of colliders in the spatial grid for broadphase collision check
		std::vector<u32> items;
	};

	struct SpatialGrid
	{
		f32 cellSize{}; //avg size of colliders
		int bucketCount = 256; //for now
		std::vector<Cell> bucket;

		SpatialGrid()
		{
			cellSize = { 1.f };
			bucketCount = 256;
			bucket.resize(bucketCount);
		}
	} _grid;

	struct PairHash
	{
		size_t operator()(u64 key) const noexcept
		{
			// Mix the bits a bit to reduce clustering
			key ^= key >> 33;
			key *= 0xff51afd7ed558ccdULL;
			key ^= key >> 33;
			return static_cast<size_t>(key);
		}
	};

	std::unordered_set<u64, PairHash> _activeTriggerPairs;

	inline CellCoord WorldToCell(float2 p, float cellSize)
	{
		return {
			static_cast<s32>(floor(p.x / cellSize)),
			static_cast<s32>(floor(p.y / cellSize))
		};
	}

	inline u32 Hash(u32 x, u32 y)
	{
		u32 h = x * 73856093u ^ y * 19349663u;
		return h;
	}

	inline u32 GetIndex(float2 p, const SpatialGrid& grid)
	{
		CellCoord coord = WorldToCell(p, grid.cellSize);
		return Hash(coord.x, coord.y) & grid.bucketCount - 1;
	}

	inline u32 GetIndex(CellCoord p, const SpatialGrid& grid)
	{
		return Hash(p.x, p.y) & grid.bucketCount - 1;
	}

	inline void ClearGrid(SpatialGrid& grid)
	{
		for (auto& cell : grid.bucket)
			cell.items.clear();
	}
}

namespace //OBB
{
	void UpdateOBBs()
	{
		for (auto col : _colliders)
		{
			if (!col) continue;
			auto box = dynamic_cast<BoxCollider*>(col);
			auto& t = box->transform();
			auto& obb = col->obb;
			f32 rad = t.rotation * (PI / 180.f); // degrees -> radians
			float2 axisX = { cosf(rad),  sinf(rad) };
			float2 axisY = { -sinf(rad), cosf(rad) };

			obb.center = t.position;
			obb.halfExtents = (box->size * t.scale) * 0.5f;
			obb.axisX = axisX;
			obb.axisY = axisY;
		}
	}

	void ProjectOBB(const OBB& obb, float2 axis, f32& outMin, f32& outMax)
	{
		f32 c = dot(obb.center, axis);

		// Project half extents onto axis (always positive contribution)
		f32 r = fabsf(dot(obb.axisX * obb.halfExtents.x, axis))
			+ fabsf(dot(obb.axisY * obb.halfExtents.y, axis));

		outMin = c - r;
		outMax = c + r;
	}

	f32 GetOverlap(const OBB& a, const OBB& b, float2 axis)
	{
		f32 aMin, aMax, bMin, bMax;
		ProjectOBB(a, axis, aMin, aMax);
		ProjectOBB(b, axis, bMin, bMax);

		// Overlap = how much they intersect
		return std::min(aMax, bMax) - std::max(aMin, bMin);
	}

	void GetOBBCorners(const OBB& obb, float2 out[4])
	{
		float2 ex = obb.axisX * obb.halfExtents.x;
		float2 ey = obb.axisY * obb.halfExtents.y;

		out[0] = obb.center + ex + ey;
		out[1] = obb.center - ex + ey;
		out[2] = obb.center - ex - ey;
		out[3] = obb.center + ex - ey;
	}

	bool IsPointInsideOBB(float2 point, const OBB& obb)
	{
		float2 d = point - obb.center;
		f32 px = fabsf(dot(d, obb.axisX));
		f32 py = fabsf(dot(d, obb.axisY));
		return px <= obb.halfExtents.x + 1e-4f
			&& py <= obb.halfExtents.y + 1e-4f;
	}

	u32 FindContactPoints(const OBB& a, const OBB& b, float2 outPoints[4])
	{
		u32 count = 0;

		float2 cornersA[4], cornersB[4];
		GetOBBCorners(a, cornersA);
		GetOBBCorners(b, cornersB);

		for (int i = 0; i < 4 && count < 4; i++)
			if (IsPointInsideOBB(cornersA[i], b))
				outPoints[count++] = cornersA[i];

		for (int i = 0; i < 4 && count < 4; i++)
			if (IsPointInsideOBB(cornersB[i], a))
				outPoints[count++] = cornersB[i];

		return count;
	}

	bool OBBvsOBB(BoxCollider& boxA, BoxCollider& boxB, ContactManifold& manifold)
	{
		OBB a = boxA.obb;
		OBB b = boxB.obb;

		// 4 axes to test: 2 from A, 2 from B
		float2 axes[4] = { a.axisX, a.axisY, b.axisX, b.axisY };

		f32    minOverlap = FLT_MAX;
		float2 minAxis = {};

		for (int i = 0; i < 4; i++)
		{
			float2 axis = axes[i];

			// Axis must be normalised - it already is since it comes from cos/sin
			f32 overlap = GetOverlap(a, b, axis);

			if (overlap <= 0.f)
				return false; // Separating axis found - no collision

			if (overlap < minOverlap)
			{
				minOverlap = overlap;
				minAxis = axis;
			}
		}

		// Make sure normal points from b -> a (pushes a out)
		float2 d = a.center - b.center;
		if (dot(d, minAxis) < 0.f)
			minAxis = minAxis * -1.f;

		// Fill manifold
		manifold.c1 = &boxA;
		manifold.c2 = &boxB;
		manifold.normal = minAxis;
		manifold.penetration = minOverlap;

		// Find contact points (cap at 2 for manifold)
		float2 allPoints[4];
		u32 count = FindContactPoints(a, b, allPoints);

		manifold.contactPointCount = std::min(count, 2u);
		for (u32 i = 0; i < manifold.contactPointCount; i++)
			manifold.contactPoints[i] = allPoints[i];

		return true;
	}

	bool RayVsOBB(float2 rayOrigin, float2 rayDir, const OBB& obb, f32& outT)
	{
		// Transform ray into OBB local space by projecting onto its axes
		float2 d = rayOrigin - obb.center;

		// Local origin and direction components along each OBB axis
		f32 ox = dot(d, obb.axisX);
		f32 oy = dot(d, obb.axisY);
		f32 dx = dot(rayDir, obb.axisX);
		f32 dy = dot(rayDir, obb.axisY);

		f32 tMin = 0.f;        // start of ray
		f32 tMax = FLT_MAX;    // end of ray

		// Test each slab (axisX and axisY)
		// A slab is the space between two parallel planes of the OBB
		auto TestSlab = [&](f32 o, f32 d, f32 halfExtent) -> bool
			{
				if (fabsf(d) < 1e-8f)
				{
					// Ray is parallel to slab — check if origin is inside
					if (fabsf(o) > halfExtent) return false;
				}
				else
				{
					f32 invD = 1.f / d;
					f32 t1 = (-halfExtent - o) * invD;
					f32 t2 = (halfExtent - o) * invD;

					if (t1 > t2) std::swap(t1, t2);

					tMin = std::max(tMin, t1);
					tMax = std::min(tMax, t2);

					if (tMin > tMax) return false; // missed
				}
				return true;
			};

		if (!TestSlab(ox, dx, obb.halfExtents.x)) return false;
		if (!TestSlab(oy, dy, obb.halfExtents.y)) return false;

		outT = tMin; // closest hit distance along ray
		return true;
	}
}


namespace
{
	//helpers
	bool CheckMask(u32 mask, u32 layer)
	{
		return mask & layer;
	}

	//wrapped function steps 
	void IntegrateMotion(f32 dt)
	{
		PROFILE_SCOPE(__func__);

		for (auto rb : _rigidbodies)
		{
			//if is an unmoving but want to prevent phasing
			if (!rb || rb->isStatic) continue;

			if (rb->useGravity)
			{
				rb->accumulatedForce += {0, -Physics::gravity };
			}

			auto& t = rb->transform();

			//integrate force
			// [A = F/M]
			rb->velocity += (rb->accumulatedForce / 1.f /*mass*/) * dt;
			t.position += rb->velocity * dt;

			//reset all force
			rb->accumulatedForce = {};
		}
	}

	void UpdateAABBs()
	{
		PROFILE_SCOPE(__func__);

		for (auto col : _colliders)
		{
			if (!col) continue;

			auto& obb = col->obb;
			auto& aabb = col->aabb;

			// Project OBB onto world axes to get a tight AABB
			float2 ex = obb.axisX * obb.halfExtents.x;
			float2 ey = obb.axisY * obb.halfExtents.y;

			// Half-extents of the enclosing AABB
			float2 r = {
				fabsf(ex.x) + fabsf(ey.x),
				fabsf(ex.y) + fabsf(ey.y)
			};

			aabb.min = obb.center - r;
			aabb.max = obb.center + r;
		}
	}

	void BuildSpatialGrid()
	{
		PROFILE_SCOPE(__func__);

		//clear grid first
		ClearGrid(_grid);

		//loop through all colliders to find aabb positions

		for (u32 i = 0; i < _colliders.size(); i++)
		{
			auto col = _colliders[i];
			if (!col) continue;

			auto& aabb = col->aabb;

			CellCoord minCell = WorldToCell(aabb.min, _grid.cellSize);
			CellCoord maxCell = WorldToCell(aabb.max, _grid.cellSize);

			for (int y = minCell.y; y <= maxCell.y; y++)
			{
				for (int x = minCell.x; x <= maxCell.x; x++)
				{
					CellCoord coord = { x,y };
					u32 j = GetIndex(coord, _grid);

					_grid.bucket[j].items.push_back(i);
				}
			}
		}
	}

	void GenerateBroadPhasePairs()
	{
		PROFILE_SCOPE(__func__);

		_broadphasePairs.clear();

		std::unordered_set<u64, PairHash> seen; //only needs to exist in this scope
		seen.reserve(1024);

		for (Cell& cell : _grid.bucket)
		{
			auto& items = cell.items;

			for (u64 i = 0; i < items.size(); i++)
			{
				for (u64 j = i + 1; j < items.size(); j++)
				{
					//check collision requirements
					auto c1 = _colliders[items[i]];
					auto c2 = _colliders[items[j]];

					if (!c1->gameObject().active() || !c2->gameObject().active()) continue;

					//check mask
					if (!CheckMask(c1->collisionMask, c2->layer)) continue;

					u32 a = std::min(items[i], items[j]);
					u32 b = std::max(items[i], items[j]);

					u64 key = (static_cast<u64>(a) << 32) | static_cast<u64>(b);

					if (seen.insert(key).second) // .second == true means it was newly inserted
						_broadphasePairs.push_back({ a, b });
				}
			}
		}
	}

	void NarrowPhaseCollision()
	{
		PROFILE_SCOPE(__func__);

		_manifolds.clear();
		//iterate through the pairs
		//we do obb on each
		for (auto& pair : _broadphasePairs)
		{
			auto* c1 = _colliders[pair.first];
			auto* c2 = _colliders[pair.second];

			if (!c1 || !c2) continue;

			// Currently only BoxColliders are supported
			auto& box1 = static_cast<BoxCollider&>(*c1);
			auto& box2 = static_cast<BoxCollider&>(*c2);

			ContactManifold manifold;
			if (!OBBvsOBB(box1, box2, manifold)) continue;

			if (c1->isTrigger || c2->isTrigger)
			{
				u32 a = std::min(pair.first, pair.second);
				u32 b = std::max(pair.first, pair.second);
				u64 key = (static_cast<u64>(a) << 32) | static_cast<u64>(b);

				if (_activeTriggerPairs.insert(key).second) // newly entered
				{
					EventHandler::RaiseEvent<OnTriggerEvent>(
						&c1->gameObject(),
						&c2->gameObject());

					EventHandler::RaiseEvent<OnTriggerEvent>(
						&c2->gameObject(),
						&c1->gameObject());
				}
				continue; // no collision response
			}


			_manifolds.push_back(manifold);

			float2 contactPoint = manifold.contactPointCount > 0
				? manifold.contactPoints[0]
				: manifold.c1->obb.center;

			// Raise for c1 (self) hit by c2 (other)
			EventHandler::RaiseEvent<OnCollisionEvent>(
				&c1->gameObject(),
				&c2->gameObject(),
				contactPoint,
				manifold.normal);

			// Raise for c2 (self) hit by c1 (other), normal flipped
			EventHandler::RaiseEvent<OnCollisionEvent>(
				&c2->gameObject(),
				&c1->gameObject(),
				contactPoint,
				-manifold.normal);

		}
	
	}

	void ResolveCollision()
	{
		PROFILE_SCOPE(__func__);

		for (auto& manifold : _manifolds)
		{
			Collider* c1 = manifold.c1;
			Collider* c2 = manifold.c2;

			//get rigidbody
			RigidBody* rb1 = c1->gameObject().GetComponent<RigidBody>();
			RigidBody* rb2 = c2->gameObject().GetComponent<RigidBody>();

			//skip if kinematic/static/null
			bool rb1_solid = rb1 && !rb1->isStatic && !rb1->isKinematic;
			bool rb2_solid = rb2 && !rb2->isStatic && !rb2->isKinematic;
			if (!rb1_solid && !rb2_solid) continue;

			//mass is 1.f for now
			const f32 mass = 1.f;
			f32 invMass1 = rb1_solid ? (1.f / mass) : 0.f;
			f32 invMass2 = rb2_solid ? (1.f / mass) : 0.f;
			f32 invMassSum = invMass1 + invMass2;
			if (invMassSum == 0.f) continue; // both infinite mass

			float2 vel1 = rb1 ? rb1->velocity : float2{};
			float2 vel2 = rb2 ? rb2->velocity : float2{};

			// normal points from c2 -> c1
			float2 normal = manifold.normal;

			//get impulse?
			float2 relativeVel = vel1 - vel2;
			f32 velAlongNormal = dot(relativeVel, normal);

			// Don't resolve if objects are already separating
			if (velAlongNormal > 0.f) goto positional_correction;

			{
				//bouciness - 0 = no bounce , 1 = full bounce
				const f32 restitution = 0.2f;

				f32 j = -(1.f + restitution) * velAlongNormal;
				j /= invMassSum;

				float2 impulse = normal * j;

				if (rb1_solid) rb1->velocity = rb1->velocity + impulse * invMass1;
				if (rb2_solid) rb2->velocity = rb2->velocity - impulse * invMass2;


				//friction
				vel1 = rb1 ? rb1->velocity : float2{};
				vel2 = rb2 ? rb2->velocity : float2{};
				relativeVel = vel1 - vel2;

				// Tangent = relative velocity minus its normal component
				float2 tangent = relativeVel - normal * dot(relativeVel, normal);
				f32 tangentLen = length(tangent);

				if (tangentLen > 1e-6f)
				{
					tangent = tangent * (1.f / tangentLen); // normalize

					f32 jt = -dot(relativeVel, tangent);
					jt /= invMassSum;

					// Coulomb's law: friction cone clamp
					// mu = coefficient of friction (tweak per material later)
					const f32 mu = 0.1f;
					float2 frictionImpulse;

					if (fabsf(jt) < j * mu)
						frictionImpulse = tangent * jt;         // static friction
					else
						frictionImpulse = tangent * (-j * mu);  // kinetic friction

					if (rb1_solid) rb1->velocity = rb1->velocity + frictionImpulse * invMass1;
					if (rb2_solid) rb2->velocity = rb2->velocity - frictionImpulse * invMass2;
				}
			}

			// -------------------------------------------------------
			// Positional correction (fixes sinking due to float error)
			// -------------------------------------------------------
		positional_correction:
			{
				// Only correct if penetration is meaningful
				const f32 slop = 0.01f;  // penetration tolerance, avoids jitter
				const f32 percentage = 0.4f;   // how aggressively to correct (0.2-0.8)

				f32 correctionMag = std::max(manifold.penetration - slop, 0.f)
					/ invMassSum * percentage;

				float2 correction = normal * correctionMag;

				if (rb1_solid)
					c1->gameObject().transform().position = c1->gameObject().transform().position + correction * invMass1;
				if (rb2_solid)
					c2->gameObject().transform().position = c2->gameObject().transform().position - correction * invMass2;
			}
		}
	}

}

namespace Physics
{
	void RegisterRigidBody(RigidBody* rb)
	{
		if (!rb) return;
		if (std::find(_rigidbodies.begin(), _rigidbodies.end(), rb) == _rigidbodies.end())
			_rigidbodies.push_back(rb);
	}

	void RegisterCollider(Collider* c)
	{
		if (!c) return;
		if (std::find(_colliders.begin(), _colliders.end(), c) == _colliders.end())
		{
			_colliders.push_back(c);
			_raycastStamp.push_back(0);
		}
	}

	void Flush()
	{
		_colliders.clear();
		_rigidbodies.clear();
		_broadphasePairs.clear();
		_manifolds.clear();
		_activeTriggerPairs.clear();
	}

	//CALL THIS AFTER REGISTERING ALL COLLIDERS!
	void Initialize()
	{
		//all colliders should be registered at this point, so we can calculate the average size for spatial grid
		UpdateOBBs();  //get obb first
		UpdateAABBs(); //derive aabb from obb

		//find average size of colliders
		f32 totalSize = 0.f;
		u32 count = 0;
		for (auto c : _colliders)
		{
			if (!c) continue;
			auto& aabb = c->aabb;
			f32 size = length(aabb.max - aabb.min);
			totalSize += size;
			count++;
		}

		_grid.cellSize = (count > 0) ? (totalSize / count) : 1.f;
		_grid.bucket.clear();
		_grid.bucket.resize(_grid.bucketCount); //set bucket size

		_broadphasePairs.reserve(1024);
	}

	void Step()
	{
		PROFILE_SCOPE("Physics");

		//Debug::ScopedTimer timer("Physics");
		f32 dt = EngineCTX::fixedDt;

		IntegrateMotion(dt);
		UpdateAABBs();
		UpdateOBBs();

		//reconstruct spatial grid
		BuildSpatialGrid();

		//broadphase collision
		/*
		* for each collider
		* -> for each other collider
		*  -> if (layerMask & collisionMask) continue;
		*  -> spatial grid check
		*/
		GenerateBroadPhasePairs();

		//narrowphase collision
		/*
		* use obb vs obb to determine collision and contact info
		*/
		NarrowPhaseCollision();

		//resolve collision
		//move objects out of collision and apply impulse
		ResolveCollision();
		//sleep objects that are at rest
	}

	bool Raycast(float2 origin, float2 direction, f32 maxDistance, RaycastHit& outHit, u32 layerMask)
	{
		f32 dirLen = length(direction);
		if (dirLen < 1e-8f) return false;
		float2 dir = direction * (1.f / dirLen);

		// Bump stamp for this raycast — all previously visited slots are now stale
		++_currentStamp;

		f32       closestT = FLT_MAX;
		Collider* closestCol = nullptr;
		u32       closestIdx = 0;

		// DDA setup — unchanged
		CellCoord cell = WorldToCell(origin, _grid.cellSize);

		s32 stepX = (dir.x >= 0.f) ? 1 : -1;
		s32 stepY = (dir.y >= 0.f) ? 1 : -1;

		f32 tDeltaX = (fabsf(dir.x) > 1e-8f) ? fabsf(_grid.cellSize / dir.x) : FLT_MAX;
		f32 tDeltaY = (fabsf(dir.y) > 1e-8f) ? fabsf(_grid.cellSize / dir.y) : FLT_MAX;

		f32 cellBoundX = (stepX > 0) ? (cell.x + 1) * _grid.cellSize : cell.x * _grid.cellSize;
		f32 cellBoundY = (stepY > 0) ? (cell.y + 1) * _grid.cellSize : cell.y * _grid.cellSize;

		f32 tMaxX = (fabsf(dir.x) > 1e-8f) ? fabsf((cellBoundX - origin.x) / dir.x) : FLT_MAX;
		f32 tMaxY = (fabsf(dir.y) > 1e-8f) ? fabsf((cellBoundY - origin.y) / dir.y) : FLT_MAX;

		f32 tCurrent = 0.f;

		while (tCurrent <= maxDistance)
		{
			u32   bucketIdx = GetIndex(cell, _grid) % static_cast<u32>(_grid.bucketCount);
			Cell& currentCell = _grid.bucket[bucketIdx];

			for (u32 idx : currentCell.items)
			{
				if (!_colliders[idx])                          continue;
				if (!_colliders[idx]->gameObject().active())   continue;
				if (!(_colliders[idx]->layer & layerMask))     continue;
				if (_colliders[idx]->isTrigger)                continue;

				// Stamp check — O(1), no allocation
				if (_raycastStamp[idx] == _currentStamp)       continue; // already visited
				_raycastStamp[idx] = _currentStamp;                      // mark visited

				f32 t = 0.f;
				if (!RayVsOBB(origin, dir, _colliders[idx]->obb, t)) continue;
				if (t < 0.f || t > maxDistance)                      continue;
				if (t >= closestT)                                    continue;

				closestT = t;
				closestCol = _colliders[idx];
			}

			if (closestCol && tCurrent > closestT) break;

			if (tMaxX < tMaxY)
			{
				tCurrent = tMaxX;
				tMaxX += tDeltaX;
				cell.x += stepX;
			}
			else
			{
				tCurrent = tMaxY;
				tMaxY += tDeltaY;
				cell.y += stepY;
			}
		}

		if (!closestCol) return false;

		// Fill RaycastHit — unchanged
		float2 hitPoint = origin + dir * closestT;
		const OBB& obb = closestCol->obb;
		float2 localHit = hitPoint - obb.center;
		f32 projX = dot(localHit, obb.axisX);
		f32 projY = dot(localHit, obb.axisY);

		float2 hitNormal;
		if (fabsf(fabsf(projX) - obb.halfExtents.x) < fabsf(fabsf(projY) - obb.halfExtents.y))
			hitNormal = obb.axisX * (projX > 0.f ? 1.f : -1.f);
		else
			hitNormal = obb.axisY * (projY > 0.f ? 1.f : -1.f);

		outHit.collider = closestCol;
		outHit.point = hitPoint;
		outHit.normal = hitNormal;
		outHit.distance = closestT;
		outHit.layerHit = closestCol->layer;

		return true;
	}
}