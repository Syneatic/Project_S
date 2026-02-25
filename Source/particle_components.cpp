#include <cmath>
#include <cstdint>
#include "ImGUI/imgui.h"

#include "json_parser_helper.hpp"

#include "gameobject.hpp"
#include "physics.hpp"
#include "particle_components.hpp"

void ParticleEmitter::OnStart() 
{
	transform = _owner->GetComponent<Transform>();
}

void ParticleEmitter::OnUpdate() 
{
	if (isBurst)
	{
		if (AEInputCheckTriggered(AEVK_G))
		{
			Burst();
		}
		return;
	}


	timer += AEFrameRateControllerGetFrameTime();
	float interval = 1.0f / spawnRate;

	//spawn particles
	while (timer >= interval)
	{
		float2 pos = transform->position;
		float rot = transform->rotation;

		//rotate to get direction
		float baseAngle = rot * (PI / 180.0f);
		float randomOffset = ((rand() % 1000 / 1000.0f) - 0.5f) * spread * (PI / 180.0f);
		float finalAngle = baseAngle + randomOffset;

		float2 velocity = {
			cosf(finalAngle) * speed,
			sinf(finalAngle) * speed
		};

		ParticleSystem::Emit(pos, velocity, lifetime, color,true,spawnRate/2,nullptr);
		timer -= interval;
	}
}

void ParticleEmitter::OnDestroy() {};

void ParticleEmitter::Burst()
{
	float2 origin = _owner->GetComponent<Transform>()->position;
	int burstCount = static_cast<int>(spawnRate);
	float angleStep = (2.0f * PI) / burstCount;

	for (int i = 0; i < burstCount; ++i)
	{
		float currentAngle = i * angleStep;
		float2 velocity = { cosf(currentAngle) * speed, sinf(currentAngle) * speed};

		ParticleSystem::Emit(origin, velocity, lifetime, color,true, burstCount/4,nullptr);
	}
}

void ParticleEmitter::Serialize(Json::Value& outComp) const
{
	outComp["spawnRate"] = spawnRate;
	outComp["speed"] = speed;
	outComp["spread"] = spread;
	outComp["lifetime"] = lifetime;

	outComp["isBurst"] = isBurst;
	outComp["color"] = WriteColor(color);
}

void ParticleEmitter::Deserialize(const Json::Value& compObj)
{
	if (compObj.isMember("spawnRate") && compObj["spawnRate"].isNumeric())
		spawnRate = compObj["spawnRate"].asFloat();

	if (compObj.isMember("speed") && compObj["speed"].isNumeric())
		speed = compObj["speed"].asFloat();

	if (compObj.isMember("spread") && compObj["spread"].isNumeric())
		spread = compObj["spread"].asFloat();

	if (compObj.isMember("lifetime") && compObj["lifetime"].isNumeric())
		lifetime = compObj["lifetime"].asFloat();

	if (compObj.isMember("isBurst") && compObj["isBurst"].isBool())
		isBurst = compObj["isBurst"].asBool();

	if (compObj.isMember("color"))
		ReadColor(compObj["color"], color);
}

void ParticleEmitter::DrawInInspector()
{
	ImGui::SeparatorText("Emitter Properties");
	ImGui::TextUnformatted("Spawn Rate");
	ImGui::DragFloat("##emitter_spawnrate", &spawnRate, 0.1f, 0, 512);
	ImGui::TextUnformatted("Burst");
	ImGui::Checkbox("##emitter_burst", &isBurst);
	ImGui::Spacing();

	ImGui::SeparatorText("Particle Properties");
	ImGui::TextUnformatted("Particle Speed");
	ImGui::DragFloat("##emitter_speed", &speed, 0.1f, 0, 512);
	ImGui::TextUnformatted("Particle Spread");
	ImGui::DragFloat("##emitter_spread", &spread, 0.1f, 0, 360);
	ImGui::TextUnformatted("Particle Lifetime");
	ImGui::InputFloat("##emitter_lifetime",&lifetime,0.01f,1.f);
	ImGui::TextUnformatted("Particle Color");
	float c[4]{color.r,color.g,color.b,color.a};
	ImGui::ColorEdit4("##emitter_color", c);
	color = Color(c);
}