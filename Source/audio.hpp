/*
Author: Yan Chun
Co-Author: Nil
*/
#pragma once

#include "audio_components.hpp"

namespace Audio
{
	void RegisterEmitter(AudioEmitter* emitter);
	void FlushEmitters();
	void Update();

	sf::SoundBuffer* LoadAudio(std::string fileName);
	void UnloadAll();
	bool HasBuffer(std::string fileName);

	void SetMasterVolume(f32 vol);
	void SetSFXVolume(f32 vol);
	void SetMusicVolume(f32 vol);
	f32 GetMasterVolume();
	f32 GetSFXVolume();
	f32 GetMusicVolume();

	void RegisterMusic(MusicPlayer* player);
	void UnregisterMusic();
	void PlayMusic();
	void StopMusic();
}