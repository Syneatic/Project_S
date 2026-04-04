/*
Author: Yan Chun
Co-Author: Nil
*/
#pragma once

#include "scene.hpp"
#include "gizmos.hpp"


namespace Editor
{
	inline std::vector<GameObject*> selectedObjects{};

	inline Transform copiedTransform;
	inline Component* copiedComponent{ nullptr };

	void SaveScene(Scene& scene);
	void DrawUI(EditorScene& escene,Scene& scene);
	void DrawGizmos();
}