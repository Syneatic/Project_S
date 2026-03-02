#pragma once

#include "scene.hpp"
#include "gizmos.hpp"

//some wrapper functions to build our editor

namespace Editor
{
	inline std::vector<GameObject*> selectedObjects{};
	void DrawUI(EditorScene& escene,Scene& scene);
}