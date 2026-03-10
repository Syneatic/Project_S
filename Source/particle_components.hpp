#pragma once

//use this more as a generic particle generator
//for atmosphere, etc...
class ParticleEmitter : public Component
{
public:
	enum class SpawnShape { POINT,RECT,CIRCLE,LINE };
	SpawnShape spawnShape = SpawnShape::POINT;

	//rect half-extent
	float2 rect{ 50.f,50.f };

	//circ
	float2 radius{ 0.f,50.f };

	//line
	float spawnLineLength = 100.f;

	float spawnRate = 10.f; // particle/sec
	float timer = 0.f;
	bool isBurst = false; //one time emit

	float time{ 0.f };
	bool timeScale{ false };
	float2 speed{ 150.f,250.f };
	float2 lifetime{ 1.f,3.f };
	float2 size{ 3.f,8.f };
	float2 spread{ -15.f,15.f };
	float2 rotation{ 0.f,0.f };

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
	void CopyFrom(Component* src) override;
	std::unique_ptr<Component> Clone(GameObject& go) override;

private:
	float2 SampleSpawnPosition() const;

};