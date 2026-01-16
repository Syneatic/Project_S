#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>


#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "scene.hpp"
#include "gameobject.hpp"
#include "component.hpp"

// parse a scene object into a scene file
// read it back from the scene file into object on load

namespace SceneIO
{
    static std::string defaultPath = "OUT/";


}