#include <list>
#include <string>
#include <iostream>
#include <unordered_map>

#include "SFML/Audio.hpp"

#include "audio.hpp"

namespace
{
	std::string assetPath{ "Assets/" };

	std::unordered_map<std::string, std::string> _musicMap{}; //<id,path>
	std::unordered_map<std::string, sf::SoundBuffer> _soundMap{};
	std::list<sf::Sound> _activeSounds{}; //prevents moving memory arnd and efficient deletion
	sf::Music _activeMusic{};

	float _masterVolume = 1.f;
	float _sfxVolume = 1.f;
	float _musicVolume = 1.f;
}

namespace Audio
{

	void LoadAudio(std::string fileName, std::string id)
	{
		sf::SoundBuffer buffer;
		if (buffer.loadFromFile(assetPath + fileName))
		{
			_soundMap[id] = buffer;
		}
	}

	void UnloadAudio(std::string id)
	{
	
	}

	void PlayAudio(std::string id, float vol,float pitch, float pan)
	{
		if (_soundMap.find(id) == _soundMap.end())
		{
			std::cout << "UNABLE TO FIND " + id + " IN BUFFER!\n";
			return;
		}


		//get buffer
		auto& buffer = _soundMap[id];
		_activeSounds.emplace_back(buffer); //create an instance and place back
		auto& sound = _activeSounds.back();
		sound.setVolume(vol);
		sound.setPitch(pitch);
		sound.setPan(pan);
		sound.play();
	}

	void Update()
	{
		//clean up dead audio
		_activeSounds.remove_if(
			[](const sf::Sound& sf)
			{
				return sf.getStatus() == sf::SoundSource::Status::Stopped;
			}
		);

		std::cout << "ACTIVE SOUNDS : " << _activeSounds.size() << std::endl;

		for (auto& s : _activeSounds)
		{
			s.setVolume(100.f * _masterVolume * _sfxVolume);
		}

		_activeMusic.setVolume(100.f * _masterVolume * _musicVolume);
	}

	void UnloadAll()
	{
		//stop all audio
		for (auto& s : _activeSounds)
		{
			s.stop();
		}

		//clear all active
		_activeSounds.clear();

		//unload from memory
		_soundMap.clear();

		_activeMusic.stop();
	}
}