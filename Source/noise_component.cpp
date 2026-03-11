#include "gameobject.hpp"
#include "particle.hpp"
#include "physics.hpp"
#include "audio_components.hpp"
#include "noise_component.hpp"
#include "event.hpp"
#include "physics_types.hpp"


void Collision(float2& pos, float2& vel,float& time, float& lifetime, Color& col, bool& shouldCollide)
{
	if (!shouldCollide) return;

	f32 dt = static_cast<f32>(EngineCTX::dt);
	uint32_t mask = (1 << 1) | (1 << 2);

	float speed = length(vel);
	if (speed < 0.001f) return;

	float2 dir = normalize(vel);
	float sweepDist = speed * dt;

	//prevent large delta time from affecting sweep
	constexpr float MAX_SWEEP = 1000.f;
	sweepDist = std::min(sweepDist, MAX_SWEEP);

	RaycastHit hit;
	if (Physics::Raycast(pos, dir, sweepDist, hit, mask))
	{
		//reflect particle
		float2 refl = reflect(dir, hit.normal);
		//add some variation
		float2 noise = float2(Random::RandFloat(-0.005f, 0.005f), Random::RandFloat(-0.005f, 0.005f));

		float2 burstVel = normalize(refl + noise) * speed;

		ParticleSystem::Emit({ hit.point + (hit.normal * 0.01f), burstVel, 0.f,
			lifetime * 0.85f, col, true, Collision });

		/*if (burstLimit > 0)
			ParticleSystem::Emit(hit.point + (hit.normal * 0.01f), burstVel, 0.f,
				lifetime * 0.85f, col, true, burstLimit / 2, Collision);*/

		//current particle becomes stationary
		pos = hit.point;
		vel = float2::zero();
		shouldCollide = false;

		//reset the time so it remains to reveal the hit
		time = 0.f;
		lifetime = 5.f;

		//change color based on what got hit
		if (hit.layerHit == (1 << 1)) col = Color(1.f, 1.f, 1.f); // environment
		if (hit.layerHit == (1 << 2)) col = Color(1.f, 0.f, 0.f); // enemy
	}

}

float GetLifetime(float noise)
{
	return noise / 10.f;
}

void NoiseSource::OnStart() 
{
	//get references
	audioEmitter = _owner.GetComponent<AudioEmitter>();
	if (!audioEmitter) throw std::runtime_error("NoiseSource requires AudioEmitter component");

	//calculate lifetime
	lifetime = GetLifetime(noiseLevel);

	if (isNoiseActive) {
		_owner.Subscribe<OnCollisionEvent, GameObject*>(
			&OnCollisionEvent::self,
			&_owner,
			[this](const OnCollisionEvent& e)
			{
				this->HandleHit(e);
			});
	}
}

void NoiseSource::OnUpdate() 
{
	if (!repeat) return;

	repeatTimer += EngineCTX::dt;

	if (repeatTimer >= repeatInterval)
	{
		repeatTimer = 0.0f;
		Emit();
	}
}

void NoiseSource::OnDestroy() 
{
	//unsubscribe event here
}

void NoiseSource::Emit()
{
	float angleStep = (2.0f * PI) / numParticles;
	for (int i = 0; i < numParticles; i++)
	{
		float currentAngle = i * angleStep;
		currentAngle += Random::RandFloat(-(angleStep/1.5f), angleStep/1.5f);
		float2 velocity = { cosf(currentAngle) , sinf(currentAngle) };
		velocity = normalize(velocity) * speed;
		//ParticleSystem::Emit(_transform.position, velocity,0.f, lifetime, color, true,Collision);
		ParticleSystem::Emit({ _transform.position, velocity,0.f, lifetime, color, true, Collision});
	}

	if (audioEmitter) audioEmitter->Play();
}

//Overload the Emit
void NoiseSource::Emit(const float2& emitPos)
{
	float angleStep = (2.0f * PI) / numParticles;
	for (int i = 0; i < numParticles; i++)
	{
		float currentAngle = i * angleStep;
		currentAngle += Random::RandFloat(-(angleStep / 1.5f), angleStep / 1.5f);
		float2 velocity = { cosf(currentAngle) , sinf(currentAngle) };
		velocity = normalize(velocity) * speed;
		ParticleSystem::Emit({ emitPos, velocity, 0.f, lifetime, color, true, Collision });
	}

	if (audioEmitter) audioEmitter->Play();
}

void NoiseSource::HandleHit(const OnCollisionEvent& e)
{
	RigidBody* rb1 = e.self->GetComponent<RigidBody>();
	RigidBody* rb2 = e.other->GetComponent<RigidBody>();
	float2 vel1 = rb1 ? rb1->velocity : float2{};
	float2 vel2 = rb2 ? rb2->velocity : float2{};
	float relativeSpeed = length(vel1 - vel2);

	//need to change
	//only emit if the collision is strong enough, prevents noise from small collisions
	if (relativeSpeed > 300.0f)
	{
		Debug::Log(relativeSpeed);
		this->Emit();
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

	const std::array<const char*, 6> layerNames[] =
	{
		"Nothing",
		"Player",
		"Environment",
		"Enemy",
		"Projectile",
		"CheckPoint"
	};

	ImGui::TextUnformatted("Collision Mask");
	if (ImGui::BeginCombo("##collisionmask", "Collision Mask"))
	{
		for (int i = 0; i < layerNames->size(); i++)
		{
			u32 bit = (i == 0) ? 0u : (1u << (i - 1));
			bool selected = (i == 0) ? (layerMask == 0) : (layerMask & bit) != 0;

			if (ImGui::Selectable(layerNames->data()[i], selected, ImGuiSelectableFlags_NoAutoClosePopups))
			{
				if (i == 0)
					layerMask = 0;
				else
					layerMask ^= bit;
			}

			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}


	ImGui::SeparatorText("Noise Properties");

	ImGui::TextUnformatted("Is Noise Active");
	ImGui::Checkbox("##isNoiseActive", &isNoiseActive);

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

	outComp["isNoiseActive"] = isNoiseActive;
	outComp["noiseLevel"] = noiseLevel;
	outComp["repeat"] = repeat;
	outComp["repeatInterval"] = repeatInterval;

	outComp["layerMask"] = layerMask;
}

void NoiseSource::Deserialize(const Json::Value& compObj)
{
	if (compObj.isMember("numParticles") && compObj["numParticles"].isNumeric())
		numParticles = compObj["numParticles"].asInt();

	if (compObj.isMember("lifetime") && compObj["lifetime"].isNumeric())
		lifetime = compObj["lifetime"].asFloat();

	if (compObj.isMember("color"))
		ReadColor(compObj["color"], color);


	if (compObj.isMember("isNoiseActive") && compObj["isNoiseActive"].isBool())
		isNoiseActive = compObj["isNoiseActive"].asBool();

	if (compObj.isMember("noiseLevel") && compObj["noiseLevel"].isNumeric())
		noiseLevel = compObj["noiseLevel"].asFloat();

	if (compObj.isMember("repeat") && compObj["repeat"].isBool())
		repeat = compObj["repeat"].asBool();

	if (compObj.isMember("repeatInterval") && compObj["repeatInterval"].isNumeric())
		repeatInterval = compObj["repeatInterval"].asFloat();

	if (compObj.isMember("layerMask")) 
		layerMask = compObj["layerMask"].asUInt();
}

void NoiseSource::CopyFrom(Component* src)
{
	auto s = dynamic_cast<NoiseSource*>(src);
	if (!s) return;

	numParticles = s->numParticles;
	speed = s->speed;
	lifetime = s->lifetime;
	color = s->color;

	//properties
	isNoiseActive = s->isNoiseActive;
	noiseLevel = s->noiseLevel;
	repeat = s->repeat;
	repeatInterval = s->repeatInterval;
	repeatTimer = s->repeatTimer;
	layerMask = s->layerMask;
}

std::unique_ptr<Component> NoiseSource::Clone(GameObject& go)
{
	auto n = std::make_unique<NoiseSource>(go);
	n.get()->CopyFrom(this);
	return n;
}
