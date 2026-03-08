#pragma once

#include "scene.hpp"
#include "gizmos.hpp"

//some wrapper functions to build our editor

namespace Editor
{
	inline std::vector<GameObject*> selectedObjects{};

	inline Transform copiedTransform;
	inline Component* copiedComponent{ nullptr };
	//inline std::type_index copiedComponentType;

	void SaveScene(Scene& scene);
	void DrawUI(EditorScene& escene,Scene& scene);
	void DrawGizmos();
}