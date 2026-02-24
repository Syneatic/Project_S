#pragma once

#include <vector>
#include "particle.hpp"
#include "base_components.hpp"

const int RAY_COUNT = 256;

//forward decl
struct Collider;
struct PlayerController;

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

	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;
	void DrawInInspector() override;
	void TriggerPing();

private:
	Collider* GetSelfCollider();
	void Ping();
	void DrawHits();
};

struct ParticleEmitter2 : Behaviour
{
	float spawnRate = 10.f; // particle/sec
	float speed = 200.f;
	float spread = 30.f;
	float lifetime = 2.f;
	float timer = 0.f;

	bool isBurst = false;
	Color color{};

	Transform* transform{ nullptr };

	void OnStart() override;
	void OnUpdate() override;

	void OnDestroy() override;

	void Burst();

	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;
	void DrawInInspector() override;

	const std::string name() const override { return "ParticleEmitter2"; }
};