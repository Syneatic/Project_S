#pragma once

#include <vector>
#include <optional>
#include "eventhandler.hpp"
#include "base_components.hpp"
#include "color.hpp"

struct AudioEmitter;

//controls audio and particle
//more specialized particle effect
struct NoiseSource : Behaviour
{
	//particle properties
	int numParticles{128};
	float speed{200.f};
	float lifetime{2.f};
	Color color{};
	//audio properties
	AudioEmitter* audioEmitter { nullptr };

	//properties
	Transform* transform { nullptr };
	float noiseLevel{}; //determines how far the noise particles will spread from the source
	bool repeat{};
	float repeatInterval{ 1.f };
	float repeatTimer{ 0.f };

	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;

	void Emit();

	const std::string name() const override { return "NoiseSource"; }

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

private:
	std::vector<std::optional<EventHandler::SubscriptionHandle>> emitterSH;
};