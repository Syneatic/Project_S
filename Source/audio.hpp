#pragma once

#include <string>

namespace Audio
{
	void LoadAudio(std::string fileName, std::string id);
	void PlayAudio(std::string id, float vol = 100.f, float pitch = 1.f, float pan = 0.f);
	void Update();
	void UnloadAll();
}