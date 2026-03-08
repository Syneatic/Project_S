#pragma once
#include "gameobject.hpp"

class AudioEmitter : public Component
{
public:
	//sf::Sound _sound;
	std::string fileName{};

	std::unique_ptr<sf::Sound> soundPtr{nullptr};

	f32 volume{1};		//  0    to 1
	f32 pitch{1};		//  0.5  to 2
	bool loop{false};
	bool spatialize{ false };
	bool relativeToListener{ false};

	void OnStart() override;
	void SetVolume(f32 vol);
	void SetPitch(f32 pitch);
	void SetLoop(bool loop);

	void Play();

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

	const std::string name() const override { return "AudioEmitter"; }

	AudioEmitter(GameObject& go) : Component(go) {};

	void CopyFrom(Component* src) override;
	std::unique_ptr<Component> Clone(GameObject& go) override;
};

class AudioListener : public Component
{
public:
	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override {};

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

	const std::string name() const override { return "AudioListener"; }

	AudioListener(GameObject& go) : Component(go) {};

	void CopyFrom(Component* src) override;
	std::unique_ptr<Component> Clone(GameObject& go) override;
};