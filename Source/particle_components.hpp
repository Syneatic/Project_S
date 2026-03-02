#pragma once

//use this more as a generic particle generator
//for atmosphere, etc...
class ParticleEmitter : public Component
{
public:
	enum class SpawnShape { POINT,RECT,CIRCLE,LINE };
	SpawnShape spawnShape = SpawnShape::POINT;

	//rect half-extent
	float spawnRectW = 50.f;
	float spawnRectH = 50.f;

	//circ
	float spawnRadiusMin = 0.f; 
	float spawnRadiusMax = 50.f;

	//line
	float spawnLineLength = 100.f;


	float spawnRate = 10.f; // particle/sec
	float timer = 0.f;
	bool isBurst = false; //one time emit

	float speedMin = 150.f; float speedMax = 250.f;
	float lifetimeMin = 1.f;   float lifetimeMax = 3.f;
	float sizeMin = 3.f;   float sizeMax = 8.f;
	float spreadMin = -15.f; float spreadMax = 15.f;


	float rotationMin = 0.f;   float rotationMax = 0.f;

	Color colorA{};
	Color colorB{};


	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;

	void Burst();

	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;
	void DrawInInspector() override;

	const std::string name() const override { return "ParticleEmitter"; }

	ParticleEmitter(GameObject& go) : Component(go) {};

private:
	float2 SampleSpawnPosition() const;

};