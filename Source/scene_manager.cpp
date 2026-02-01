#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <memory>

#include "scene_parser.hpp"
#include "scene.hpp"
#include "scene_editor.hpp"

namespace
{
	namespace fs = std::filesystem;

	std::vector<std::string> _sceneRegistry{}; //hold all scene in 'Scene' folder

	Scene* _current;
	std::string _nextSceneName;
	bool _requestSwitch = false;
	bool _requestReload = false;
	EditorScene _editor{};

	bool InSceneRegistry(std::string name)
	{
		return std::find(_sceneRegistry.begin(), _sceneRegistry.end(), name) != _sceneRegistry.end();
	}

	bool HasExtension(const fs::path& p, std::string ext)
	{
		std::string e = p.extension().string();
		auto lower = [](unsigned char c) { return (char)std::tolower(c); };
		std::transform(e.begin(), e.end(), e.begin(), lower);
		std::transform(ext.begin(), ext.end(), ext.begin(), lower);
		return e == ext;
	}

	Scene* LoadSceneFromDisk(const std::string& name)
	{
		Scene* scn = new Scene(name);
		SceneIO::DeserializeScene(*scn, name);
		return scn;
	}

	void SwitchScene()
	{
		_requestSwitch = false;
		if (_nextSceneName.empty()) return;

		if (_current) _current->OnExit();
		delete _current;                      // unload old scene NOW

		_current = LoadSceneFromDisk(_nextSceneName);
		if (_current) _current->OnEnter();

		_nextSceneName.clear();
		_requestReload = false;
	}

	void ReloadScene()
	{
		_requestReload = false;
		if (!_current) return;

		const std::string name = _current->name(); // you already have name()
		_current->OnExit();
		delete _current;

		_current = LoadSceneFromDisk(name);
		if (_current) _current->OnEnter();
	}
}

namespace SceneManager
{
	void Initialize()
	{
		//read all scene files in a folder
		_sceneRegistry.clear();
		const fs::path sceneFolder = "Scene";

		std::error_code ec;

		if (!fs::exists(sceneFolder, ec) || !fs::is_directory(sceneFolder, ec))
		{
			std::cout << "Scene folder not found!" << std::endl;
			return; //folder missing (or log an error)
		}

		for (const auto& entry : fs::directory_iterator(sceneFolder, ec))
		{
			if (ec) break;

			if (!entry.is_regular_file(ec))
				continue;

			const fs::path& p = entry.path();

			//only grab .scene files
			if (!HasExtension(p, ".scene"))
				continue;

			//filename w/o extension
			_sceneRegistry.emplace_back(p.stem().string());
		}

		//for now we sort by alphabetical order
		std::sort(_sceneRegistry.begin(), _sceneRegistry.end());
		std::cout << _sceneRegistry.size() << std::endl;
		for (std::string s : _sceneRegistry)
		{
			std::cout << s << std::endl;
		}
	}

	void SwitchToEditor()
	{
		_requestSwitch = false;

		if (_current) _current->OnExit();
		delete _current;                      // unload old scene NOW
		EditorScene* e = new EditorScene;
		e->imguiInitialized = true;
		_current = e;
		if (_current) _current->OnEnter();

		_nextSceneName.clear();
		_requestReload = false;
	}

	void RequestSceneSwitch(const std::string& sceneName)
	{
		//check if exists
		if (!InSceneRegistry(sceneName)) return;

		_nextSceneName = sceneName;
		_requestSwitch = true;
	}

	void RequestSceneReload() { _requestReload = true; }

	void OnUpdate()
	{
		if (_requestSwitch) SwitchScene();
		if (_requestReload) ReloadScene();
		if (_current) _current->OnUpdate();
	}
}
