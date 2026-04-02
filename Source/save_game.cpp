
#include "save_game.hpp"

namespace SaveGameManager
{
	static const std::string savePath = "Saves/";

	void Save(const SaveData& data)
	{

		std::filesystem::create_directories(savePath);

		std::string fullPath = savePath + data.sceneName + ".save";

		Json::Value root;

		//save the specific data value
		root["scene"] = data.sceneName;
		root["firstSpawn"] = WriteFloat2(data.initialSpawnPosition);
		root["spawnPoint"] = WriteFloat2(data.spawnPoint);
		root["timer"] = data.timer;

		Json::StreamWriterBuilder builder; //Json writer to transfer from the program to the new file
		builder["indentation"] = " ";

		std::ofstream out(fullPath, std::ios::binary);
		if (!out) return; //File cannot be read

		std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
		writer->write(root, &out);
	}

	void Load(const std::string& sceneName, SaveData& outData)
	{
		std::string fullPath = savePath + sceneName + ".save";
		
		//Debug::Log("Level Save Name", fullPath), "\n";

		std::ifstream in(fullPath, std::ios::binary);
		if (!in) return; //check if input file stream cannot be read

		Json::CharReaderBuilder builder;
		std::string errs;
		Json::Value root;

		if (!parseFromStream(builder, in, &root, &errs))
			return;

		if (root.isMember("scene"))
			outData.sceneName = root["scene"].asString();

		if (root.isMember("firstSpawn"))
			ReadFloat2(root["firstSpawn"], outData.initialSpawnPosition);

		if (root.isMember("spawnPoint"))
			ReadFloat2(root["spawnPoint"], outData.spawnPoint);

		if (root.isMember("timer"))
			outData.timer = root["timer"].asFloat();

		//File has been successfully run
		//return true;
	}
};
