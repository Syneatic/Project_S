#pragma once

#include <vector>
#include "particle.hpp"
#include "base_components.hpp"

struct ParticleEmitter : Behaviour
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

	const std::string name() const override { return "ParticleEmitter"; }
};