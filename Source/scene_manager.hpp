/*
Author: Yan Chun
Co-Author: Nil
*/
#pragma once

class Scene;

namespace SceneManager
{
	void Initialize();
	void SwitchToEditor();
	void RequestSceneSwitch(const std::string& sceneName);
	void RequestSceneReload();
	void OnUpdate();
	void QuitApplication();
	Scene* ActiveScene();
}
