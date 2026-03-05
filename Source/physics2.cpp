#include "physics2.hpp"
#include "gameobject.hpp"
#include "physics_components.hpp"

namespace 
{
	std::vector<Collider*> _colliders;
	std::vector<RigidBody*> _rigidbodies;

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
		f32 cellSize; //avg size of colliders
		int bucketCount = 256; //for now
		std::vector<Cell> bucket;
	} _grid;


	inline CellCoord WorldToCell(float2 p, float cellSize)
	{
		return {
			static_cast<u32>(floor(p.x / cellSize)),
			static_cast<u32>(floor(p.y / cellSize))
		};
	}

	inline u32 Hash(u32 x, u32 y)
	{
		u32 h = x * 73856093u ^ y * 19349663u;
		return h;
	}

	inline u32 GetIndex(float2 p,const SpatialGrid& grid)
	{
		CellCoord coord = WorldToCell(p, grid.cellSize);
		return Hash(coord.x, coord.y) & grid.bucketCount;
	}

	inline u32 GetIndex(CellCoord p,const SpatialGrid& grid)
	{
		return Hash(p.x, p.y) & grid.bucketCount;
	}

	inline void ClearGrid(SpatialGrid& grid)
	{
		for (auto& cell : grid.bucket)
			cell.items.clear();
	}

	void IntegrateMotion(f32 dt)
	{
		for (auto rb : _rigidbodies)
		{
			//if is an unmoving but want to prevent phasing
			if (!rb || rb->isStatic) continue;

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
		for (auto col : _colliders)
		{
			if (!col) continue;

			//assumption right now is all colliders are box
			auto& b = static_cast<BoxCollider&>(*col);
			auto& t = col->transform();
			auto& aabb = col->aabb;
			
			aabb.min = t.position - (b.size * t.scale) * 0.5f;
			aabb.max = t.position + (b.size * t.scale) * 0.5f;
		}
	}

	void BuildSpatialGrid()
	{
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
		if (std::find(_rigidbodies.begin(), _rigidbodies.end(), c) == _rigidbodies.end())
			_colliders.push_back(c);
	}

	//CALL THIS AFTER REGISTERING ALL COLLIDERS!
	void Initialize()
	{
		//all colliders should be registered at this point, so we can calculate the average size for spatial grid
		UpdateAABBs(); //get aabb first

		//find average size of colliders
		f32 totalSize = 0.f;
		u32 count = 0;
		for (auto c :_colliders)
		{
			if (!c) continue;
			auto& aabb = c->aabb;
			f32 size = length(aabb.max - aabb.min);
			totalSize += size;
			count++;
		}

		_grid.cellSize = (count > 0) ? (totalSize / count) : 1.f;
		
		_grid.bucket.resize(_grid.bucketCount); //set bucket size
	}

	void Step()
	{
		f32 dt = EngineCTX::dt;

		IntegrateMotion(dt);
		UpdateAABBs();

		//reconstruct spatial grid

		//broadphase collision
		/*
		* for each collider
		* -> for each other collider
		*  -> if (layerMask & collisionMask) continue;
		*  -> spatial grid check
		*/

		//narrowphase collision
		/*
		* use obb vs obb to determine collision and contact info
		*/

		//resolve collision
		//move objects out of collision and apply impulse

		//sleep objects that are at rest
	}
}