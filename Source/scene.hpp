#pragma once
#include <vector>
#include "gameobject.hpp"


//maybe not

struct Scene
{
protected:
	std::vector<GameObject> _gameObjects{};
	char* sceneName{};

public:
	virtual void InitializeScene()
	{
		//read from file
		//initialize all gameobjects
		/*
		* for each game objects
		*	=> gameobjects.Start()
		*/
	}

	virtual void UpdateScene()
	{

	}

	virtual void DestroyScene()
	{
		//delete
	}
};