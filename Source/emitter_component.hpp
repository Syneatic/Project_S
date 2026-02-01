#pragma once
#include <vector>
#include <cmath>
#include <cstdint>
#include <string>
#include "ImGUI/imgui.h"

#include "component.hpp"
#include "collider_components.hpp"
#include "physics.hpp"
#include "particle.hpp"
#include "color.hpp"

#include "AEGraphics.h"
#include "AEInput.h"
#include "AEFrameRateController.h"

static const int RAY_COUNT = 256;

struct ParticleEmitter : Behaviour
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
	Color color{1.f,1.f,1.f,1.f};

	// State
	std::vector<Hit> hits{};
	float2 origin{};
	bool   active = false;
	float  t0 = 0.0f;
	float  time = 0.0f;
	float timeLimit = 10.0f;

	const std::string name() const override { return "ParticleEmitter"; }

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
		if (AEInputCheckTriggered(pingKey)) {
			Ping();
		}
		if (time >= timeLimit) {
			Ping();
			time = 0.0f;
		}

		// draw revealed points
		DrawHits();
	}

	void OnDestroy() override
	{
		hits.clear();
	}

	void Serialize(Json::Value& outComp) const override
	{
		outComp["color"] = WriteColor(color);
	}

	void Deserialize(const Json::Value& compObj) override
	{
		if (compObj.isMember("color"))
			ReadColor(compObj["color"], color);
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

		uint32_t raycastMask = 0xFFFFFFFF & ~static_cast<uint32_t>(Layer::Player);
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
			if (Physics::Raycast(origin, dir, maxRadius, rh, raycastMask))
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

			RenderSystem::DrawPoint(par.pos, color);
		}
	}
};

struct WaterEmitter : ParticleEmitter {
	WaterEmitter() {
		rayCount = 20;
		maxRadius = 50.f;
		speed = 100.f;
		color = {0,0,1.f,1.f};
		timeLimit = 3.0f;
    }
    const std::string name() const override { return "WaterEmitter"; }
};

struct EditableEmitter : ParticleEmitter {
	void DrawInInspector() override{
		ImGui::TextUnformatted("Color");
		float col[4] = { color.r, color.g, color.b, color.a };
		if (ImGui::ColorEdit4("###renderer_color", col))
		{
			color.r = col[0];
			color.g = col[1];
			color.b = col[2];
			color.a = col[3];
		}
	}

	EditableEmitter() {
		rayCount = 20;
		maxRadius = 50.f;
		speed = 100.f;
		timeLimit = 3.0f;
	}
    const std::string name() const override { return "EditableEmitter"; }
};