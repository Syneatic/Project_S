#include "gameobject.hpp"
#include "physics.hpp"
#include "particle_components.hpp"
#include "particle.hpp"

void ParticleEmitter::OnStart() 
{

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


	timer += static_cast<float>(EngineCTX::dt);
	float interval = 1.0f / spawnRate;

	while (timer >= interval)
	{
		float2 spawnPos = SampleSpawnPosition();

		float baseAngle;
		if (spawnShape == SpawnShape::LINE)
		{
			// Line runs along local X, so perpendicular is local Y = rot + 90deg
			baseAngle = (_transform.rotation + 90.f) * (PI / 180.0f);
		}
		else
		{
			baseAngle = _transform.rotation * (PI / 180.0f);
		}

		float spread = Random::RandFloat(spreadMin, spreadMax) * (PI / 180.0f);
		float angle = baseAngle + spread;
		float spd = Random::RandFloat(speedMin, speedMax);

		float2 velocity = { cosf(angle) * spd, sinf(angle) * spd };
		float  life = Random::RandFloat(lifetimeMin, lifetimeMax);
		float  size = Random::RandFloat(sizeMin, sizeMax);
		float  rotation = Random::RandFloat(rotationMin, rotationMax);
		Color  col = RandColor(colorA,colorB);

		ParticleSystem::Emit(spawnPos, velocity, 0.f, life, col,
			true, static_cast<int>(spawnRate / 2),
			nullptr, size, rotation);

		timer -= interval;
	}
}

void ParticleEmitter::OnDestroy() {};

void ParticleEmitter::Burst()
{
	float2 origin = _transform.position;
	int    count = static_cast<int>(spawnRate);
	float  angleStep = (2.0f * PI) / count;

	for (int i = 0; i < count; ++i)
	{
		float2 spawnPos = SampleSpawnPosition();
		float  angle = i * angleStep;
		float  spd = Random::RandFloat(speedMin, speedMax);
		float2 velocity = { cosf(angle) * spd, sinf(angle) * spd };
		float  life = Random::RandFloat(lifetimeMin, lifetimeMax);
		float  size = Random::RandFloat(sizeMin, sizeMax);
		float  rotation = Random::RandFloat(rotationMin, rotationMax);
		Color  col = RandColor(colorA,colorB);

		ParticleSystem::Emit(spawnPos, velocity, 0.f, life, col,
			true, count / 4,
			nullptr, size, rotation);
	}
}

void ParticleEmitter::Serialize(Json::Value& outComp) const
{
	outComp["spawnShape"] = static_cast<int>(spawnShape);
	outComp["spawnRectW"] = spawnRectW;
	outComp["spawnRectH"] = spawnRectH;
	outComp["spawnRadiusMin"] = spawnRadiusMin;
	outComp["spawnRadiusMax"] = spawnRadiusMax;
	outComp["spawnLineLength"] = spawnLineLength;

	outComp["spawnRate"] = spawnRate;
	outComp["isBurst"] = isBurst;

	outComp["speedMin"] = speedMin;    outComp["speedMax"] = speedMax;
	outComp["lifetimeMin"] = lifetimeMin; outComp["lifetimeMax"] = lifetimeMax;
	outComp["sizeMin"] = sizeMin;     outComp["sizeMax"] = sizeMax;
	outComp["spreadMin"] = spreadMin;   outComp["spreadMax"] = spreadMax;
	outComp["rotationMin"] = rotationMin; outComp["rotationMax"] = rotationMax;

	outComp["colorA"] = WriteColor(colorA);
	outComp["colorB"] = WriteColor(colorB);
}

void ParticleEmitter::Deserialize(const Json::Value& o)
{
	auto readFloat = [&](const char* key, float& val) {
		if (o.isMember(key) && o[key].isNumeric()) val = o[key].asFloat();
		};

	if (o.isMember("spawnShape") && o["spawnShape"].isInt())
		spawnShape = static_cast<SpawnShape>(o["spawnShape"].asInt());

	readFloat("spawnRectW", spawnRectW);
	readFloat("spawnRectH", spawnRectH);
	readFloat("spawnRadiusMin", spawnRadiusMin);
	readFloat("spawnRadiusMax", spawnRadiusMax);
	readFloat("spawnLineLength", spawnLineLength);
	readFloat("spawnRate", spawnRate);

	if (o.isMember("isBurst") && o["isBurst"].isBool())
		isBurst = o["isBurst"].asBool();

	readFloat("speedMin", speedMin);    readFloat("speedMax", speedMax);
	readFloat("lifetimeMin", lifetimeMin); readFloat("lifetimeMax", lifetimeMax);
	readFloat("sizeMin", sizeMin);     readFloat("sizeMax", sizeMax);
	readFloat("spreadMin", spreadMin);   readFloat("spreadMax", spreadMax);
	readFloat("rotationMin", rotationMin); readFloat("rotationMax", rotationMax);

	if (o.isMember("colorA")) ReadColor(o["colorA"], colorA);
	if (o.isMember("colorB")) ReadColor(o["colorB"], colorB);
}

void ParticleEmitter::DrawInInspector()
{
	ImGui::SeparatorText("Spawn Shape");
	const char* shapeNames[] = { "Point", "Rectangle", "Circle", "Line" };
	int shapeIdx = static_cast<int>(spawnShape);
	if (ImGui::Combo("##spawnshape", &shapeIdx, shapeNames, 4))
		spawnShape = static_cast<SpawnShape>(shapeIdx);

	switch (spawnShape)
	{
	case SpawnShape::RECT:
		ImGui::TextUnformatted("Half Width");  ImGui::DragFloat("##rectW", &spawnRectW, 1.f, 0, 2000);
		ImGui::TextUnformatted("Half Height"); ImGui::DragFloat("##rectH", &spawnRectH, 1.f, 0, 2000);
		break;
	case SpawnShape::CIRCLE:
		ImGui::TextUnformatted("Inner Radius"); ImGui::DragFloat("##radMin", &spawnRadiusMin, 1.f, 0, 2000);
		ImGui::TextUnformatted("Outer Radius"); ImGui::DragFloat("##radMax", &spawnRadiusMax, 1.f, 0, 2000);
		break;
	case SpawnShape::LINE:
		ImGui::TextUnformatted("Length"); ImGui::DragFloat("##lineLen", &spawnLineLength, 1.f, 0, 2000);
		break;
	default: break;
	}

	ImGui::SeparatorText("Emission");
	ImGui::TextUnformatted("Spawn Rate"); ImGui::DragFloat("##spawnrate", &spawnRate, 0.1f, 0, 512);
	ImGui::TextUnformatted("Burst Mode"); ImGui::Checkbox("##burst", &isBurst);

	ImGui::SeparatorText("Particle Properties");

	ImGui::TextUnformatted("Speed (min / max)");
	ImGui::DragFloat("##spdMin", &speedMin, 1.f, 0, 2000); ImGui::SameLine();
	ImGui::DragFloat("##spdMax", &speedMax, 1.f, 0, 2000);

	ImGui::TextUnformatted("Lifetime (min / max)");
	ImGui::DragFloat("##ltMin", &lifetimeMin, 0.01f, 0, 60); ImGui::SameLine();
	ImGui::DragFloat("##ltMax", &lifetimeMax, 0.01f, 0, 60);

	ImGui::TextUnformatted("Size (min / max)");
	ImGui::DragFloat("##szMin", &sizeMin, 0.1f, 0, 200); ImGui::SameLine();
	ImGui::DragFloat("##szMax", &sizeMax, 0.1f, 0, 200);

	ImGui::TextUnformatted("Spread degrees (min / max)");
	ImGui::DragFloat("##spMin", &spreadMin, 0.1f, -360, 360); ImGui::SameLine();
	ImGui::DragFloat("##spMax", &spreadMax, 0.1f, -360, 360);

	ImGui::TextUnformatted("Initial Rotation rad (min / max)");
	ImGui::DragFloat("##rotMin", &rotationMin, 0.01f, -PI, PI); ImGui::SameLine();
	ImGui::DragFloat("##rotMax", &rotationMax, 0.01f, -PI, PI);

	ImGui::TextUnformatted("Color A");
	float ca[4]{ colorA.r, colorA.g, colorA.b, colorA.a };
	if (ImGui::ColorEdit4("##colorA", ca)) colorA = Color(ca);

	ImGui::TextUnformatted("Color B");
	float cb[4]{ colorB.r, colorB.g, colorB.b, colorB.a };
	if (ImGui::ColorEdit4("##colorB", cb)) colorB = Color(cb);
}


float2 ParticleEmitter::SampleSpawnPosition() const
{
	float2 origin = _transform.position;
	float  rot = _transform.rotation * (PI / 180.0f);
	float  cr = cosf(rot), sr = sinf(rot);

	// Rotate a local offset into world space
	auto toWorld = [&](float lx, float ly) -> float2 {
		return { origin.x + cr * lx - sr * ly,
				 origin.y + sr * lx + cr * ly };
		};

	switch (spawnShape)
	{
	case SpawnShape::RECT:
	{
		float lx = Random::RandFloat(-spawnRectW, spawnRectW);
		float ly = Random::RandFloat(-spawnRectH, spawnRectH);
		return toWorld(lx, ly);
	}
	case SpawnShape::CIRCLE:
	{
		// Uniform disk sampling between inner and outer radius
		float angle = Random::RandFloat(0.f, 2.f * PI);
		// sqrt for uniform area distribution
		float r = sqrtf(Random::RandFloat(
			spawnRadiusMin * spawnRadiusMin,
			spawnRadiusMax * spawnRadiusMax));
		return toWorld(cosf(angle) * r, sinf(angle) * r);
	}
	case SpawnShape::LINE:
	{
		float t = Random::RandFloat(-0.5f, 0.5f);
		float lx = t * spawnLineLength;
		return toWorld(lx, 0.f);
	}
	case SpawnShape::POINT:
	default:
		return origin;
	}
}