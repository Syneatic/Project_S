/*
Author: Tan Wei Jun
Co-Author: Nil
*/
#pragma once
#include "math.hpp"

//saveData during the game
//add changes to position of necessary gameobject
namespace SaveGameManager {
	class SaveData
	{
	public:
		std::string sceneName;
		float2 initialSpawnPosition{};
		float2 spawnPoint{};
		f32 timer{};
	};
	inline bool toLoad{ false };

	void Save(const SaveData& data);
	void Load(const std::string& sceneName, SaveData& outData);
}