#pragma once
#include "scene.hpp"
#include "scene_editor.hpp"

//Debugger Scene = Read Only Scene
//Open Scene, Render and allow user
//to inspect game properties in scene
class DebuggerScene:public Scene
{
public:
	void OnEnter() override;
	void OnUpdate() override;
	void OnExit() override;

	void LoadScene(const std::string& sceneName);

private:
	Scene _loadedScene{};
	GameObject* _hoveredObject{ nullptr };
};

namespace Debugger
{
	GameObject* PickHoveredObject(Scene& scene);
	void DrawUI(DebuggerScene& dscene, Scene& scene, GameObject* hovered);
}