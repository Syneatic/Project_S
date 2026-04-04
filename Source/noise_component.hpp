/*
Author: Yan Chun
Co-Author: Wei Jun
*/
#pragma once

#include "eventhandler.hpp"
#include "color.hpp"

class AudioEmitter;

//controls audio and particle
//more specialized particle effect
class NoiseSource : public Component
{
public:
	//particle properties
	int numParticles{128};
	float speed{400.f}; //keep standard to this speed
	float lifetime{2.f}; //determines how far the particle can travel
	Color color{};
	//audio properties
	AudioEmitter* audioEmitter { nullptr };

	//properties
	bool isNoiseActive{true};
	float noiseLevel{}; //determines how far the noise particles will spread from the source
	bool repeat{};
	float repeatInterval{ 1.f };
	float repeatTimer{ 0.f };
	u32 layerMask{};

	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;

	void Emit();
	void Emit(const float2& emitPos);
	void HandleHit(const OnCollisionEvent& e);

	const std::string name() const override { return "NoiseSource"; }

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

	NoiseSource(GameObject& go) : Component(go) {};
	void CopyFrom(Component* src) override;
	std::unique_ptr<Component> Clone(GameObject& go) override;
};