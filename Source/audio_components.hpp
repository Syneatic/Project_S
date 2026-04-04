/*
Author: Yan Chun
Co-Author: Nil
*/
#pragma once
#include "gameobject.hpp"

// emits a sound at the position of the gameobject, can be spatialized and have volume/pitch/loop settings
class AudioEmitter : public Component
{
public:
	//sf::Sound _sound;
	std::string fileName{};

	std::unique_ptr<sf::Sound> soundPtr{nullptr};

	f32 volume{1};		//  0    to 1
	f32 pitch{1};		//  0.5  to 2
	bool loop{false};
	bool spatialize{ true };
	bool relativeToListener{ false};

	//initializes sound
	void OnStart() override;

	//sets volume/pitch/loop values
	void SetVolume(f32 vol);
	void SetPitch(f32 pitch);
	void SetLoop(bool loop);

	//plays sound
	void Play();
	//plays a given sound clip
	void Play(sf::Sound clip);

	//displays sound properties in inspector and allows editing them
	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

	const std::string name() const override { return "AudioEmitter"; }

	AudioEmitter(GameObject& go) : Component(go) {};

	void CopyFrom(Component* src) override;
	std::unique_ptr<Component> Clone(GameObject& go) override;
};

// represents the position of the listener in the scene, used for spatialization of audio emitters
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

// plays a music file
class MusicPlayer : public Component
{
public:
	std::string fileName{};
	f32 volume{ 1 };		//  0    to 1
	bool loop{ false };

	void OnStart() override;
	void OnDestroy() override;

	void SetVolume(f32 vol);
	void SetLoop(bool loop);

	void Play();
	void Stop();

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

	const std::string name() const override { return "MusicPlayer"; }
	MusicPlayer(GameObject& go) : Component(go) {};
	void CopyFrom(Component* src) override;
	std::unique_ptr<Component> Clone(GameObject& go) override;
};