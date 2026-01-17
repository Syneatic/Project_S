#pragma once
#include <vector>
#include <string>
#include "gameobject.hpp"

//maybe not
struct Scene
{
protected:
	std::vector<std::unique_ptr<GameObject>> _gameObjectList{};
	std::string _name{};

public:
	virtual void Initialize()
	{
		//read from file
		//initialize all gameobjects
		/*
		* for each game objects
		*	=> gameobjects.Start()
		*/
	}

	virtual void Update()
	{

	}

	virtual void Destroy()
	{
		//delete
	}

	//===== SERIALIZATION =====
	const std::string& name() const { return _name; }
	std::string& name(std::string name) { return _name = std::move(name); }
	std::vector<std::unique_ptr<GameObject>>& gameObjectList() { return _gameObjectList; }

	Scene() {}
	Scene(std::string name) { _name = std::move(name); }
};

//singleton
//maybe just make functions in a file
struct SceneManager
{
private:
	static Scene* currentScene;
public:
	void SetCurrentScene(Scene* scene)
	{
		currentScene = scene;
	}

	void InitializeCurrentScene()
	{
		if (currentScene)
			currentScene->Initialize();
	}

	void UpdateCurrentScene()
	{
		if (currentScene)
			currentScene->Update();
	}

	void DestroyCurrentScene()
	{
		if (currentScene)
			currentScene->Destroy();
	}
};