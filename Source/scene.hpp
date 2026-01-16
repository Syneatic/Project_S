#pragma once
#include <vector>
#include <string>
#include "gameobject.hpp"

//maybe not
struct Scene
{
protected:
	std::vector<std::unique_ptr<GameObject>> gameObjectList{};
	std::string name{};

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
	virtual std::string TypeId() const = 0;
	virtual void Serialize() {}
	virtual void Deserialize() {}
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