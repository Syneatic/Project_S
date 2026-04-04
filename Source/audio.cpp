/*
Author: Yan Chun
Co-Author: Zachary Yee
*/
#include "audio.hpp"

namespace
{
	std::string assetPath{ "Assets/" };

	//ref to all audio emitters
	std::vector<AudioEmitter*> _audioEmitters{};
	std::unordered_set<AudioEmitter*> _audioEmitterSet;

	std::unordered_map<std::string, std::string> _musicMap{}; //<id,path>
	std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> _soundBufferMap{}; //<filename,buffer>
	MusicPlayer* _activeMusicPlayer{ nullptr };
	std::unique_ptr<sf::Music> _activeMusic{};

	float _masterVolume = 1.f;
	float _sfxVolume = 1.f;
	float _musicVolume = 1.f;
}

namespace Audio
{
	sf::SoundBuffer* LoadAudio(std::string fileName)
	{
		sf::SoundBuffer buffer;
		if (buffer.loadFromFile(assetPath + fileName))
		{
			_soundBufferMap[fileName] = std::make_unique<sf::SoundBuffer>(buffer);
			return _soundBufferMap[fileName].get();
		}

		return nullptr;
	}

	void UnloadAll()
	{
		for (auto e : _audioEmitters)
		{
			e->soundPtr.get()->stop();
		}
		//unload from memory
		_soundBufferMap.clear();

		UnregisterMusic();
	}

	bool HasBuffer(std::string fileName)
	{
		return _soundBufferMap.find(fileName) != _soundBufferMap.end();
	}

	void RegisterEmitter(AudioEmitter* e)
	{
		if (!e) return;

		if (_audioEmitterSet.insert(e).second)
		{
			_audioEmitters.push_back(e);

			sf::SoundBuffer* buffer{ nullptr };
			if (_soundBufferMap.find(e->fileName) == _soundBufferMap.end())
				//create an audio buffer from file
				buffer = LoadAudio(e->fileName);
			else
				//get existing buffer
				buffer = _soundBufferMap[e->fileName].get();

			if (!buffer)
			{
				Debug::Log(__LINE__,__FILE__,"AUDIO BUFFER IS NULL");
				return;
			}

			e->soundPtr = std::make_unique<sf::Sound>(*buffer);
			if (!e->soundPtr)
				Debug::Log("ERROR MAKING SOUND BUFFER");
		}
	}

	void UnregisterEmitter(AudioEmitter* e)
	{
		if (!e) return;
		if (_audioEmitterSet.erase(e) == 0) return;

		//find the renderer
		auto it = std::find(_audioEmitters.begin(), _audioEmitters.end(), e);
		if (it != _audioEmitters.end())
		{
			//push to back and pop
			*it = _audioEmitters.back();
			_audioEmitters.pop_back();

			//unload audio
		}
	}

	void FlushEmitters()
	{
		_audioEmitters.clear();
		_audioEmitterSet.clear();
	}

	void Update()
	{
		for (auto* emitter : _audioEmitters)
		{
			auto& transform = emitter->transform();
			auto& sound = *emitter->soundPtr.get();

			sound.setVolume(100.f * emitter->volume * _sfxVolume * _masterVolume);
			sound.setPitch(emitter->pitch);

			//update position
			sf::Vector3f pos(transform.position.x, transform.position.y, 0.f);
			sound.setPosition(pos);
			auto dir = sound.getPosition() - sf::Listener::getPosition();	
		}

		if (_activeMusic)
		{
			_activeMusic->setVolume(100.f * _activeMusicPlayer->volume * _musicVolume * _masterVolume);
			_activeMusic->setLooping(_activeMusicPlayer->loop);
		}
	}

	void SetMasterVolume(f32 vol)
	{
		_masterVolume = std::clamp(vol, 0.f, 1.f);
	}

	void SetSFXVolume(f32 vol)
	{
		_sfxVolume = std::clamp(vol, 0.f, 1.f);
	}

	void SetMusicVolume(f32 vol)
	{
		_musicVolume = std::clamp(vol, 0.f, 1.f);
	}

	f32 GetMasterVolume()
	{
		return _masterVolume;
	}

	f32 GetSFXVolume()
	{
		return _sfxVolume;
	}

	f32 GetMusicVolume()
	{
		return _musicVolume;
	}

	// Set music to be relative to listener.
	void RegisterMusic(MusicPlayer* player)
	{
		if (!player) return;

		_activeMusicPlayer = player;
		_activeMusic = std::make_unique<sf::Music>(assetPath + player->fileName);
		_activeMusic.get()->setRelativeToListener(true);
		_activeMusic.get()->setPosition({ 0.f, 0.f, 0.f });
		_activeMusic.get()->setAttenuation(0.f);
	}

	void UnregisterMusic()
	{
		_activeMusic->stop();
		_activeMusic.release();
	}

	void PlayMusic()
	{
		if (_activeMusic->getStatus() != sf::SoundSource::Status::Playing)
			_activeMusic->play();
	}

	void StopMusic()
	{
		if (_activeMusic->getStatus() == sf::SoundSource::Status::Playing)
			_activeMusic->stop();
	}
}