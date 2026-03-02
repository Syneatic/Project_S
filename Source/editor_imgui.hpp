#pragma once

#include "scene.hpp"
#include "gizmos.hpp"

//some wrapper functions to build our editor

namespace Editor
{
	inline std::vector<int> selectedIndices{};
	void DrawUI(EditorScene& escene,Scene& scene);
}