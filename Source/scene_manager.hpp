#pragma once

#include <string>

namespace SceneManager
{
	void Initialize();
	void SwitchToEditor();
	void RequestSceneSwitch(const std::string& sceneName);
	void RequestSceneReload();
	void OnUpdate();
}
