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
	};

	void Save(const SaveData& data);
	bool Load(const std::string& sceneName, SaveData& outData);
}