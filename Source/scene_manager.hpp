#pragma once

namespace SceneManager
{
	void Initialize(int* gameLoop);
	void SwitchToEditor();
	void RequestSceneSwitch(const std::string& sceneName);
	void RequestSceneReload();
	void OnUpdate();
	void QuitApplication();
}
