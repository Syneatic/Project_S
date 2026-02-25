#include <vector>

#include "IMGUI/imgui.h"
#include "json_parser_helper.hpp"

#include "math.hpp"
#include "gameobject.hpp"
#include "particle.hpp"
#include "physics.hpp"
#include "audio_components.hpp"
#include "noise_component.hpp"

namespace
{
	enum PingEventID
	{
		AUDIO,
		EMIT
	};
}

void Collision(float2& pos, float2& vel,float& time, float& lifetime, Color& col, bool& shouldCollide, int& burstLimit)
{
	float dt = AEFrameRateControllerGetFrameTime();
	uint32_t mask = 1 << 1;
	mask |= 1 << 2;
	mask |= 1 << 3;

	if (shouldCollide)
	{
		float2 velocity = vel * dt;
		float speed = length(vel);
		float dist = length(velocity);
		if (dist > 0.001f) //if particle not moving, skip raycast
		{
			float2 dir = normalize(vel);
			Physics::RaycastHit hit;

			if (Physics::Raycast(pos, dir, dist, hit, mask))
			{
				//version1
				/*
				if (burstLimit > 16)
				{
					float centerAngle = atan2f(hit.normal.y, hit.normal.x);

					float arcRange = PI;
					float angleStep = arcRange / (burstLimit - 1);

					float startAngle = centerAngle - (arcRange / 2.0f);
					
					for (int i = 0; i < burstLimit; i++)
					{
						float currentAngle = startAngle + (i * angleStep);
						currentAngle += Random::RandFloat(-(angleStep / 4.0f), angleStep / 4.0f);
						float2 b_velocity = { cosf(currentAngle) , sinf(currentAngle) };
						b_velocity = normalize(b_velocity) * speed;
						ParticleSystem::Emit(hit.point + (hit.normal * 0.01f), b_velocity, lifetime * 0.6f, col, true, burstLimit/ 2, Collision);
					}
				}
				*/
				
				//get reflected vector
				float2 refl = reflect(dir,hit.normal);
				float2 b_velocity = (normalize(refl) + float2(Random::RandFloat(-0.005f, 0.005f),Random::RandFloat(-0.005f,0.005f))) * speed;
				ParticleSystem::Emit(hit.point + (hit.normal * 0.01f), b_velocity,0.f, lifetime * 0.85f, col, true, burstLimit / 2, Collision);



				//reset lifetime when hit
				time = 0.f;
				lifetime = 5.f;
				
				shouldCollide = false;
				if (hit.layerHit == (1 << 1))
					col = Color(1.0f, 1.0f, 1.0f);

				if (hit.layerHit == (1 << 2))
					col = Color(1.0f, 0.0f, 0.0f);
				
				pos = hit.point;
				//kill momentum
				vel = float2::zero();
			}
		}
	}
}

float GetLifetime(float noise)
{
	return noise / 10.f;
}

void NoiseSource::OnStart() 
{
	//get references
	transform = _owner->GetComponent<Transform>();
	if (!transform) throw std::runtime_error("NoiseSource requires Transform component");

	audioEmitter = _owner->GetComponent<AudioEmitter>();
	if (!audioEmitter) throw std::runtime_error("NoiseSource requires AudioEmitter component");
	
	//calculate lifetime
	lifetime = GetLifetime(noiseLevel);

	emitterSH.push_back(SubscribeFilter(&PingEvent::targetId, _owner->id, [this]()
	{
		audioEmitter->Play();
	}));

	emitterSH.push_back(SubscribeFilter(&PingEvent::targetId, _owner->id, [this]()
	{
		Emit();
	}));
}

void NoiseSource::OnUpdate() 
{
	if (!repeat) return;

	float dt = AEFrameRateControllerGetFrameTime();
	repeatTimer += dt;

	if (repeatTimer >= repeatInterval)
	{
		EventHandler::RaiseEvent<PingEvent>(_owner->id, AUDIO);
		EventHandler::RaiseEvent<PingEvent>(_owner->id, EMIT);
		repeatTimer = 0.0f;
	}
}

void NoiseSource::OnDestroy() 
{
	for (auto& e : emitterSH)
	{
		EventHandler::Unsubscribe(e.value());
	}
}

void NoiseSource::Emit()
{
	float angleStep = (2.0f * PI) / numParticles;
	for (int i = 0; i < numParticles; i++)
	{
		float currentAngle = i * angleStep;
		currentAngle += Random::RandFloat(-(angleStep/1.5), angleStep/1.5);
		float2 velocity = { cosf(currentAngle) , sinf(currentAngle) };
		velocity = normalize(velocity) * speed;
		ParticleSystem::Emit(transform->position, velocity,0.f, lifetime, color, true, numParticles/2,Collision);
	}
}



void NoiseSource::DrawInInspector() 
{
	lifetime = GetLifetime(noiseLevel);
	ImGui::SeparatorText("Particle Properties");
	ImGui::TextUnformatted("Particle Count");
	ImGui::DragInt("##noise_particlecount", &numParticles,1,0);
	ImGui::TextUnformatted("Particle Lifetime");
	ImGui::TextDisabled("%.2f", lifetime);
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