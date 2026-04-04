/*
Author: Yan Chun
Co-Author: Wei Jun, Harith, Zachary Yee
*/
#include "gameobject.hpp"
#include "physics.hpp"
#include "particle_components.hpp"
#include "particle.hpp"

void ParticleEmitter::OnStart() 
{
	//warms up the emitter
	float interval = 1.0f / spawnRate;
	float simulatedTime = time;

	//simulate backwards in time to emit already aged particles at the start
	while (simulatedTime >= interval)
	{
		float2 spawnPos = SampleSpawnPosition();

		float baseAngle;
		if (spawnShape == SpawnShape::LINE)
			baseAngle = (_worldTransform.rotation + 90.f) * (PI / 180.0f);
		else
			baseAngle = _worldTransform.rotation * (PI / 180.0f);

		float sprd = Random::RandFloat(spread.x, spread.y) * (PI / 180.0f);
		float angle = baseAngle + sprd;
		float spd = Random::RandFloat(speed.x, speed.y);
		float2 velocity = { cosf(angle) * spd, sinf(angle) * spd };
		float  life = Random::RandFloat(lifetime.x, lifetime.y);
		float  sz = Random::RandFloat(size.x, size.y);
		float  rot = Random::RandFloat(rotation.x, rotation.y);
		Color  col = RandColor(colorA, colorB);

		float age = time - simulatedTime;

		if (age < life)
		{
			float2 agedPos = {
				spawnPos.x + velocity.x * age,
				spawnPos.y + velocity.y * age
			};

			ParticleSystem::Emit(
				{ 
					agedPos, velocity, age, life, col,
					false, nullptr, sz, rot, renderLayer, 
					sortOrder, timeScale
				});
		}

		simulatedTime -= interval;
	}
}

void ParticleEmitter::OnUpdate() 
{
	if (isBurst) //only emit when triggered
	{
		if (AEInputCheckTriggered(AEVK_G))
			Burst();

		return;
	}

	timer += static_cast<float>(timeScale ? EngineCTX::dt : EngineCTX::unscaledDt);
	float interval = 1.0f / spawnRate;

	//emit particles at fixed time intervals based on spawnRate, accounting for frame time
	while (timer >= interval)
	{
		float2 spawnPos = SampleSpawnPosition();

		float baseAngle;
		if (spawnShape == SpawnShape::LINE)
		{
			// Line runs along local X, so perpendicular is local Y = rot + 90deg
			baseAngle = (_worldTransform.rotation + 90.f) * (PI / 180.0f);
		}
		else
			baseAngle = _worldTransform.rotation * (PI / 180.0f);

		float sprd = Random::RandFloat(spread.x, spread.y) * (PI / 180.0f);
		float angle = baseAngle + sprd;
		float spd = Random::RandFloat(speed.x, speed.y);

		float2 velocity = { cosf(angle) * spd, sinf(angle) * spd };
		float  life = Random::RandFloat(lifetime.x, lifetime.y);
		float  sz = Random::RandFloat(size.x, size.y);
		float  rot = Random::RandFloat(rotation.x, rotation.y);
		Color  col = RandColor(colorA,colorB);

		ParticleSystem::Emit(
			{
				spawnPos, velocity, 0.f, life, col,
				true, nullptr, sz, rot, renderLayer, 
				sortOrder, timeScale
			});

		timer -= interval;
	}
}

//particle emitted are handled by the particle system, so no cleanup needed here
void ParticleEmitter::OnDestroy() {};

void ParticleEmitter::Burst()
{
	//emit a burst of particles all at once based on spawnRate, ignoring frame time and timer
	float2 origin = _worldTransform.position;
	int    count = static_cast<int>(spawnRate);
	float  angleStep = (2.0f * PI) / count;

	for (int i = 0; i < count; ++i)
	{
		float2 spawnPos = SampleSpawnPosition();

		float  angle = i * angleStep;
		float  spd = Random::RandFloat(speed.x, speed.y);
		float2 velocity = { cosf(angle) * spd, sinf(angle) * spd };
		float  life = Random::RandFloat(lifetime.x, lifetime.y);
		float  sz = Random::RandFloat(size.x, size.y);
		float  rot = Random::RandFloat(rotation.x, rotation.y);
		Color  col = RandColor(colorA,colorB);

		ParticleSystem::Emit(spawnPos, velocity, time, life, col,
			false, nullptr, sz, rot,renderLayer,sortOrder);
	}
}

void ParticleEmitter::Serialize(Json::Value& outComp) const
{
	outComp["spawnShape"] = static_cast<int>(spawnShape);
	outComp["renderLayer"] = static_cast<int>(renderLayer);
	outComp["sortOrder"] = sortOrder;
	outComp["rect"] = WriteFloat2(rect);
	outComp["radius"] = WriteFloat2(radius);
	outComp["spawnLineLength"] = spawnLineLength;

	outComp["spawnRate"] = spawnRate;
	outComp["isBurst"] = isBurst;

	outComp["time"] = time;
	outComp["timescale"] = timeScale;
	outComp["speed"] = WriteFloat2(speed);
	outComp["lifetime"] = WriteFloat2(lifetime);
	outComp["size"] = WriteFloat2(size);
	outComp["spread"] = WriteFloat2(spread);
	outComp["rotation"] = WriteFloat2(rotation);

	outComp["colorA"] = WriteColor(colorA);
	outComp["colorB"] = WriteColor(colorB);
}

void ParticleEmitter::Deserialize(const Json::Value& o)
{
	auto readfloat2 = [&](const char* key, float2& val) 
		{
			if (o.isMember(key)) 
				ReadFloat2(o[key], val);
		};

	if (o.isMember("spawnShape") && o["spawnShape"].isInt())
		spawnShape = static_cast<SpawnShape>(o["spawnShape"].asInt());

	renderLayer = static_cast<Graphics::RenderLayer>(o["renderLayer"].asInt());
	sortOrder = o["sortOrder"].asFloat();

	readfloat2("rect", rect);
	readfloat2("radius", radius);
	spawnLineLength = o["spawnLineLength"].asFloat();
	spawnRate = o["spawnRate"].asFloat();

	if (o.isMember("isBurst") && o["isBurst"].isBool())
		isBurst = o["isBurst"].asBool();

	time = o["time"].asFloat();
	timeScale = o["timescale"].asBool();
	readfloat2("speed", speed);    
	readfloat2("lifetime", lifetime); 
	readfloat2("size", size);     
	readfloat2("spread", spread);   
	readfloat2("rotation", rotation); 

	if (o.isMember("colorA")) ReadColor(o["colorA"], colorA);
	if (o.isMember("colorB")) ReadColor(o["colorB"], colorB);
}

void ParticleEmitter::DrawInInspector()
{
	RenderSort("particle",renderLayer,sortOrder);
	ImGui::SeparatorText("Spawn Shape");

	const char* shapeNames[] = { "Point", "Rectangle", "Circle", "Line" };
	int shapeIdx = static_cast<int>(spawnShape);
	if (ImGui::Combo("##spawnshape", &shapeIdx, shapeNames, 4))
		spawnShape = static_cast<SpawnShape>(shapeIdx);

	switch (spawnShape)
	{
	case SpawnShape::RECT:
		Float2DragReset("Half Width", "particle", &rect.x, { 50.f,50.f });
		break;
	case SpawnShape::CIRCLE:
		Float2DragReset("Radius", "particle", &radius.x, { 0.f,50.f });
		break;
	case SpawnShape::LINE:
		FloatDragReset("Length", "particle", &spawnLineLength, 100.f);
		break;
	default: break;
	}

	ImGui::SeparatorText("EngineCTX");
	ImGui::Checkbox("Timescale", &timeScale);

	ImGui::SeparatorText("Emission");
	FloatDrag("Spawn Rate","particle", &spawnRate);
	ImGui::TextUnformatted("Burst Mode"); ImGui::Checkbox("##burst", &isBurst);
	FloatDragReset("Warm Up Time","particle", &time,0.f);

	ImGui::SeparatorText("Particle Properties");

	Float2DragReset("Speed [MIN | MAX]", "particle", &speed.x, { 150.f,250.f });
	Float2DragReset("Lifetime [MIN | MAX]", "particle", &lifetime.x, { 1.f,3.f });
	Float2DragReset("Size [MIN | MAX]", "particle", &size.x, { 3.f,8.f });
	Float2DragReset("Spread [MIN | MAX]", "particle", &spread.x, { 0.f,0.f });
	Float2DragReset("Rotation [MIN | MAX]", "particle", &rotation.x, { 0.f,0.f });
;
	ImGui::TextUnformatted("Color A");
	float ca[4]{ colorA.r, colorA.g, colorA.b, colorA.a };
	if (ImGui::ColorEdit4("##colorA", ca)) colorA = Color(ca);

	ImGui::TextUnformatted("Color B");
	float cb[4]{ colorB.r, colorB.g, colorB.b, colorB.a };
	if (ImGui::ColorEdit4("##colorB", cb)) colorB = Color(cb);
}

void ParticleEmitter::CopyFrom(Component* src)
{
	auto s = dynamic_cast<ParticleEmitter*>(src);
	if (!s) return;

	spawnShape = s->spawnShape;

	//rect half-extent
	rect = s->rect;

	//circ
	radius = s->radius;

	//line
	spawnLineLength = s->spawnLineLength;

	spawnRate = s->spawnRate; // particle/sec
	isBurst = s->isBurst; //one time emit

	time = s->time;
	speed = s->speed;
	lifetime = s->lifetime;
	size = s->size;
	spread = s->spread;
	rotation = s->rotation;

	colorA = s->colorA;
	colorB = s->colorB;
}

std::unique_ptr<Component> ParticleEmitter::Clone(GameObject& go)
{
	auto n = std::make_unique<ParticleEmitter>(go);
	n.get()->CopyFrom(this);
	return n;
}

float2 ParticleEmitter::SampleSpawnPosition() const
{
	float2 origin = _worldTransform.position;
	float  rot = _worldTransform.rotation * (PI / 180.0f);
	float  cr = cosf(rot), sr = sinf(rot);

	// Rotate a local offset into world space
	auto toWorld = [&](float lx, float ly) -> float2 {
		return { origin.x + cr * lx - sr * ly,
				 origin.y + sr * lx + cr * ly };
		};

	//get a random point within the defined spawn shape in local space, then convert to world space
	switch (spawnShape)
	{
	case SpawnShape::RECT:
	{
		float lx = Random::RandFloat(-rect.x, rect.y);
		float ly = Random::RandFloat(-rect.x, rect.y);
		return toWorld(lx, ly);
	}
	case SpawnShape::CIRCLE:
	{
		// Uniform disk sampling between inner and outer radius
		float angle = Random::RandFloat(0.f, 2.f * PI);
		// sqrt for uniform area distribution
		float r = sqrtf(Random::RandFloat(
			radius.x * radius.x,
			radius.y * radius.y));
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