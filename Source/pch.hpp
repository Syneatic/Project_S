#pragma once

//disable macros
#define NOMINMAX


//std
#include <algorithm>
#include <array>
#include <cmath>
#include <cfloat>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <crtdbg.h>
#include <deque>
#include <functional>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <initializer_list>
#include <list>
#include <memory>
#include <mutex>
#include <numeric>
#include <optional>
#include <queue>
#include <string>
#include <shobjidl.h> 
#include <sstream>
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
#include "math.hpp"
#include "color.hpp"
#include "transform.hpp"
#include "enginectx.hpp"
#include "base_components.hpp"

#include "debug.hpp"
#include "scene_manager.hpp"
#include "renderer.hpp"
#include "imgui_helper.hpp"
#include "json_parser_helper.hpp"

#include "profiler_ui.h"