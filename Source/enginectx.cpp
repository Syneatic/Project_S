/*
Author: Zachary Yee
Co-Author: Yan Chun
*/
#pragma once

#include "enginectx.hpp"

namespace EngineCTX
{
	void PauseTime()
	{
		isPaused = !isPaused;
		timeScale = isPaused ? 0.0f : 1.0f;
	}

	//get root path
	std::filesystem::path GetRootPath()
	{
		auto path = std::filesystem::current_path();
		//check if it exists or not check one level back
		if (!std::filesystem::exists(path / "Assets")) 
		{
			if (std::filesystem::exists(path.parent_path().parent_path() / "Assets")) 
			{
				return path.parent_path().parent_path();
			}
		}
		return path;
	}

	//return absolute path
	std::string GetAbsPath(const std::string& relativePath) 
	{
		return (GetRootPath() / relativePath).string();
	}
}