#pragma once
#include <vector>
#include <string>

#include "AEEngine.h" //temporary
#include "gameobject.hpp"
#include "renderer.hpp"

struct Scene
{
protected:
	std::vector<std::unique_ptr<GameObject>> _gameObjectList{};
	std::string _name{};

	void InitializeGameObjects()
	{
		for (auto& pgo : _gameObjectList)
		{
			auto go = pgo.get();
			for (auto& [type, comp] : go->componentMap())
			{
				/*if (auto* r = dynamic_cast<Renderer*>(comp.get()))
					RenderSystem::Instance().RegisterRenderer(r);

				if (auto* b = dynamic_cast<Behaviour*>(comp.get()))
					b->OnStart();*/
			}
		}
	}

public:
	virtual void OnEnter()
	{
		//load data from file

		//initialize all gameobjects
		if (_gameObjectList.empty()) return;
		InitializeGameObjects();
	}

	virtual void OnUpdate()
	{
		//test draw
		AEGfxSetBackgroundColor(1.f,0.f,0.f);
	}

	virtual void OnExit()
	{
		//delete
	}

	//===== SERIALIZATION =====
	const std::string& name() const { return _name; }
	const std::string& name(std::string name) { return _name = std::move(name); }
	std::vector<std::unique_ptr<GameObject>>& gameObjectList() { return _gameObjectList; }
	const std::vector<std::unique_ptr<GameObject>>& gameObjectList() const { return _gameObjectList; }


	Scene() {}
	Scene(std::string name) { _name = std::move(name); }
	virtual ~Scene() = default;
};

//singleton
//maybe just make functions in a file
struct SceneManager
{
public:
	//singleton access
	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}

	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
	SceneManager(SceneManager&&) = delete;
	SceneManager& operator=(SceneManager&&) = delete;

	
	void RequestSceneSwitch(Scene* scene)
	{
		if (!scene) return; //invalid scene

		nextScene = scene;
		requestSwitch = true;
	}

	void RequestSceneReload()
	{
		requestReload = true;
	}

	void OnUpdate()
	{
		if (requestSwitch) SwitchScene();
		if (requestReload) ReloadScene();

		if (currentScene) currentScene->OnUpdate();
	}

private:
	SceneManager() = default;
	~SceneManager() = default;

	Scene* currentScene = nullptr;
	Scene* nextScene = nullptr;
	bool requestSwitch = false;
	bool requestReload = false;

	void SwitchScene()
	{
		requestSwitch = false;
		if (!nextScene || nextScene == currentScene)
			return;

		if (currentScene) currentScene->OnExit(); //check null then exit
		currentScene = nextScene; //switch scene
		currentScene->OnEnter();

		requestReload = false; //prevent reload if switch
	}

	void ReloadScene()
	{
		requestReload = false;

		if (!currentScene)
			return;

		currentScene->OnExit();
		currentScene->OnEnter();
	}
};