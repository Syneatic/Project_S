#include "IMGUI/imgui.h"
#include "json_parser_helper.hpp"

#include "math.hpp"

#include "gameobject.hpp"
#include "particle.hpp"
#include "physics.hpp"
#include "audio_components.hpp"

#include "noise_component.hpp"

void Collision(float2& pos, float2& vel, float& lifetime, Color& col, bool& shouldCollide, int& burstLimit)
{
	float dt = AEFrameRateControllerGetFrameTime();
	uint32_t mask = 1 << 1;
	mask |= 1 << 2;
	mask |= 1 << 3;

	Physics::RaycastHit hit;
	if (Physics::Raycast(pos, vel, length(vel * dt), hit, mask))
	{
		//commented out until optimized
		//RecursiveEmit(hit,g_pool.vel[i],g_pool.lifetime[i],g_pool.burstRemaining[i],g_pool.color[i]);

		if (hit.layerHit == (1 << 1))
			col = Color(1.0f, 1.0f, 1.0f);

		if (hit.layerHit == (1 << 2))
			col = Color(1.0f, 0.0f, 0.0f);

		//kill momentum
		pos = hit.point;
		vel = float2::zero();
	}
}

void NoiseSource::OnStart() 
{
	//get references
	transform = _owner->GetComponent<Transform>();
	if (!transform) throw std::runtime_error("NoiseSource requires Transform component");

	audioEmitter = _owner->GetComponent<AudioEmitter>();
	if (!audioEmitter) throw std::runtime_error("NoiseSource requires AudioEmitter component");
}

void NoiseSource::OnUpdate() 
{
	if (!repeat) return;

	float dt = AEFrameRateControllerGetFrameTime();
	repeatTimer += dt;

	if (repeatTimer >= repeatInterval)
	{
		Emit();
		repeatTimer = 0.0f;
	}
}

void NoiseSource::OnDestroy() 
{

}

void NoiseSource::Emit()
{
	float angleStep = (2.0f * PI) / numParticles;
	for (int i = 0; i < numParticles; i++)
	{
		float currentAngle = i * angleStep;
		currentAngle += Random::RandFloat(-(angleStep/2), angleStep/2);
		float2 velocity = { cosf(currentAngle) * speed, sinf(currentAngle) * speed };
		ParticleSystem::Emit(transform->position, velocity, lifetime, color, true, 0,Collision);
	}
}



void NoiseSource::DrawInInspector() 
{
	ImGui::SeparatorText("Particle Properties");
	ImGui::TextUnformatted("Particle Count");
	ImGui::DragInt("##noise_particlecount", &numParticles,1,0);
	ImGui::TextUnformatted("Particle Speed");
	ImGui::DragFloat("##noise_particlespeed", &speed, 0.1f);
	ImGui::TextUnformatted("Particle Lifetime");
	ImGui::DragFloat("##noise_particlelife", &lifetime, 0.1f,0.f);
	ImGui::TextUnformatted("Color");
	float c[4]{ color.r,color.g,color.b,color.a };
	ImGui::ColorEdit4("##emitter_color", c);
	color = Color(c);

	ImGui::SeparatorText("Noise Properties");
	ImGui::TextUnformatted("Noise Level");
	ImGui::DragFloat("##noise_level", &noiseLevel, 1, 0);
	ImGui::TextUnformatted("Repeat?");
	ImGui::Checkbox("##noise_repeat", &repeat);
	if (repeat)
	{
		ImGui::TextUnformatted("Repeat Interval");
		ImGui::DragFloat("##noise_repeatinterval", &repeatInterval, 0.1f,0.f);
	}
}

void NoiseSource::Serialize(Json::Value& outComp) const
{
	outComp["numParticles"] = numParticles;
	outComp["speed"] = speed;
	outComp["lifetime"] = lifetime;
	outComp["color"] = WriteColor(color);

	outComp["noiseLevel"] = noiseLevel;
	outComp["repeat"] = repeat;
	outComp["repeatInterval"] = repeatInterval;
}

void NoiseSource::Deserialize(const Json::Value& compObj)
{
	if (compObj.isMember("numParticles") && compObj["numParticles"].isNumeric())
		numParticles = compObj["numParticles"].asInt();

	if (compObj.isMember("speed") && compObj["speed"].isNumeric())
		speed = compObj["speed"].asFloat();;

	if (compObj.isMember("lifetime") && compObj["lifetime"].isNumeric())
		lifetime = compObj["lifetime"].asFloat();

	if (compObj.isMember("color"))
		ReadColor(compObj["color"], color);



	if (compObj.isMember("noiseLevel") && compObj["noiseLevel"].isNumeric())
		noiseLevel = compObj["noiseLevel"].asFloat();

	if (compObj.isMember("repeat") && compObj["repeat"].isBool())
		repeat = compObj["repeat"].asBool();

	if (compObj.isMember("repeatInterval") && compObj["repeatInterval"].isNumeric())
		repeatInterval = compObj["repeatInterval"].asFloat();

}