#pragma once
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "json.h"

#include "scene.hpp"
#include "gameobject.hpp"
#include "component.hpp"
#include "math.hpp"

// parse a scene object into a scene file
// read it back from the scene file into object on load

namespace SceneIO
{
	static std::string defaultPath = "OUT/";



	void SerializeScene(/*Scene scn*/)
	{
		/*
		* 
		*/
	}

	void DeserializeScene()
	{
		/*
		* Create scene object
		*/
	}
}