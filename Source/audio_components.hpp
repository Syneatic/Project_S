#pragma once
#include <string>
#include "SFML/Audio.hpp"
#include "AETypes.h"

#include "gameobject.hpp"
#include "base_components.hpp"

#include "transform_component.hpp"

//requires transform
struct AudioEmitter : Component
{
	//sf::Sound _sound;
	std::string fileName{};

	std::unique_ptr<sf::Sound> soundPtr{nullptr};

	Transform* transform{nullptr};
	f32 volume{1};		//  0    to 1
	f32 pitch{1};		//  0.5  to 2
	bool loop{false};
	bool spatialize{ false };
	bool relativeToListener{ false};

	void Initialize();
	void SetVolume(f32 vol);
	void SetPitch(f32 pitch);
	void SetLoop(bool loop);

	void Play();

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

	const std::string name() const override { return "AudioEmitter"; }
};

struct AudioListener : Behaviour
{
	Transform* transform;

	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override {};

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

	const std::string name() const override { return "AudioListener"; }
};