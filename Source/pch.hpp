#pragma once

//std
#include <algorithm>
#include <cmath>
#include <cfloat>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <crtdbg.h>
#include <functional>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <shobjidl.h> 
#include <stdexcept>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <ostream>
#include <vector>
#include <windows.h>



//3rd
#include "IMGUI/imgui.h"
#include "ImGUI/imgui_impl_opengl3.h"
#include "ImGUI/imgui_impl_win32.h"
#include "json.h"
#include "SFML/Audio.hpp"
#include "AEEngine.h"


//our
#include "json_parser_helper.hpp"
#include "base_components.hpp"
#include "transform_component.hpp"
#include "debug.hpp"