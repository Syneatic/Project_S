#pragma once
#include <algorithm>
#include <iostream>
#include "AETypes.h"

#include "audio.hpp"
#include "SFML/Audio.hpp"

#include "gameobject.hpp"
#include "transform_component.hpp"
#include "base_components.hpp"

//requires transform
struct AudioEmitter : Behaviour
{
private:
	sf::Sound _sound;
	Transform* _transform;
	f32 _volume{1};		//  0    to 1
	f32 _pitch{1};		//  0.5  to 2
	bool _loop{false};


public:
	void SetVolume(f32 vol)
	{
		_volume = std::clamp(vol, 0.0f, 1.0f);
	}

	void SetPitch(f32 pitch)
	{
		_pitch = std::clamp(pitch, 0.5f, 2.0f);
	}

	void SetLoop(bool loop)
	{
		_loop = loop;
	}

	void OnStart() override
	{
		_transform = _owner->GetComponent<Transform>();
		if (!_transform)
		{
			std::cout << "NO TRANSFORM FOUND IN " << _owner->name() << std::endl;
		}
	}

	void OnUpdate() override
	{
		if (_transform)
		{
			sf::Vector3f pos(_transform->position.x, _transform->position.y, 0.f);
			_sound.setPosition(pos);
		}

		//_sound.setVolume();
	}
};

struct AudioListener : Behaviour
{
private:
	Transform* _transform;

public:
	void OnStart() override
	{
		_transform = _owner->GetComponent<Transform>();
		if (!_transform)
		{
			std::cout << "NO TRANSFORM FOUND IN " << _owner->name() << std::endl;
		}
	}

	void OnUpdate() override
	{
		if (_transform)
		{
			sf::Vector3f pos(_transform->position.x, _transform->position.y, 0.f);
			sf::Listener::setPosition(pos);
		}
	}
};