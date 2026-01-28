#pragma once
#include <vector>
#include <cmath>
#include <cstdint>

#include "component.hpp"
#include "collider_components.hpp"
#include "physics.hpp"
#include "particle.hpp"

#include "AEGraphics.h"
#include "AEInput.h"
#include "AEFrameRateController.h"

static const int RAY_COUNT = 256;

struct EchoPingTest : Behaviour
{
	struct Hit
	{
		float2 p{};
		float2 n{};
		float  d{};
	};

	// Tweakables
	int rayCount = RAY_COUNT;
	float maxRadius = 500.0f;   // world units
	float speed = 300.0f;   // world units per second
	u8    pingKey = 'E';      // change if you use different keycodes
	u32   pointColor = 0xFFFFFFFF;

	// State
	std::vector<Hit> hits{};
	float2 origin{};
	bool   active = false;
	float  t0 = 0.0f;
	float  time = 0.0f;

	const std::string name() const override { return "EchoPingTest"; }

	Particle particles[RAY_COUNT]{};
	bool complete = false;

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

		// draw revealed points
		DrawHits();
	}

	void OnDestroy() override
	{
		hits.clear();
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
		auto* tr = gameObject().GetComponent<Transform>();
		if (!tr) return;

		origin = tr->position;
		t0 = time;
		active = true;

		hits.clear();

		Physics::RaycastHit rh{};
		Collider* ignore = GetSelfCollider();

		const float twoPi = 6.28318530718f;
		f32 dt = AEFrameRateControllerGetFrameTime();
		float spd = speed * dt;
		for (int i = 0; i < rayCount; ++i)
		{
			float a = (static_cast<float>(i) / static_cast<float>(rayCount)) * twoPi;
			float2 dir{ std::cos(a), std::sin(a) }; // already normalized
			particles[i].pos = origin;
			particles[i].vel = dir * spd;
			particles[i].time = 0.f;
			particles[i].lifetime = maxRadius / (speed);
			particles[i].active = true;
			particles[i].stay = false;

			//cast ray
			if (Physics::Raycast(origin, dir, maxRadius, rh))
			{
				Hit h{};
				h.p = rh.point;
				h.n = rh.normal;
				h.d = rh.distance;
				hits.push_back(h);
			}
		}

		complete = false;
	}

	void DrawHits()
	{
		// only draw if alive or hit wall
		for (size_t j = 0; j < hits.size(); j++)
		{
			auto& hit = hits[j];

			for (size_t i = 0; i < RAY_COUNT; i++)
			{
				auto& par = particles[i];
				if (par.stay)
				{
					continue;
				}
				//if particle reached hit point, we draw
				if (length(hit.p - par.pos) <= length(par.vel * 1.5f))
				{
					par.pos = hit.p;
					par.stay = true;
					break;
				}
			}			
		}

		for (size_t i = 0; i < RAY_COUNT; i++)
		{
			//draw particles
			auto& par = particles[i];
			if (!par.stay)
			{
				par.pos += par.vel;
			}
			
			par.lifetime -= static_cast<float>(AEFrameRateControllerGetFrameTime());

			if (par.lifetime <= 0.0f && !par.stay)
				continue;

			RenderSystem::DrawPoint(par.pos);
		}
	}
};
