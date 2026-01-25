#pragma once
#include <vector>
#include <cmath>
#include <cstdint>

#include "component.hpp"
#include "collider_components.hpp"
#include "physics.hpp"

#include "AEGraphics.h"
#include "AEInput.h"
#include "AEFrameRateController.h"

struct EchoPingTest : Behaviour
{
	struct Hit
	{
		float2 p{};
		float2 n{};
		float  d{};
	};

	// Tweakables
	int   rayCount = 256;
	float maxRadius = 1000.0f;   // world units
	float speed = 100.0f;   // world units per second
	u8    pingKey = 'E';      // change if you use different keycodes
	u32   pointColor = 0xFFFFFFFF;

	// State
	std::vector<Hit> hits{};
	float2 origin{};
	bool   active = false;
	float  t0 = 0.0f;
	float  time = 0.0f;

	const std::string name() const override { return "EchoPingTest"; }

	void OnStart() override
	{
		hits.reserve((size_t)rayCount);
	}

	void OnUpdate() override
	{
		// advance local time
		time += static_cast<float>(AEFrameRateControllerGetFrameTime());

		// trigger ping
		if (AEInputCheckTriggered(pingKey))
			Ping();

		if (!active)
			return;

		float r = speed * (time - t0);

		// stop after wave passes
		if (r > maxRadius)
			active = false;

		// draw revealed points
		DrawHits(r);
	}

	void OnDestroy() override
	{
		hits.clear();
		active = false;
	}

	void Serialize(Json::Value& outComp) const override
	{

	}

	void Deserialize(const Json::Value& compObj) override
	{

	}

private:
	Collider* GetSelfCollider()
	{
		if (auto* c = gameObject().GetComponent<CircleCollider>()) return c;
		if (auto* b = gameObject().GetComponent<BoxCollider>())    return b;
		return nullptr;
	}

	void Ping()
	{
		hits.clear();
		auto* tr = gameObject().GetComponent<Transform>();
		if (!tr) return;

		origin = tr->position;
		t0 = time;
		active = true;

		hits.clear();

		Physics::RaycastHit rh{};
		Collider* ignore = GetSelfCollider();

		const float twoPi = 6.28318530718f;

		for (int i = 0; i < rayCount; ++i)
		{
			float a = (static_cast<float>(i) / static_cast<float>(rayCount)) * twoPi;
			float2 dir{ std::cos(a), std::sin(a) }; // already normalized

			if (Physics::Raycast(origin, dir, maxRadius, rh))
			{
				Hit h{};
				h.p = rh.point;
				h.n = rh.normal;
				h.d = rh.distance;
				hits.push_back(h);
			}
		}
	}

	void DrawHits(float currentRadius)
	{
		for (size_t i = 0; i < hits.size(); i++)
		{
			auto hit = hits[i];
			RenderSystem::DrawPoint(hit.p);
		}
	}
};
