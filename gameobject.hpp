#pragma once
#include <vector>
#include "component.hpp"

struct GameObject
{
private:
	char* _name;
	std::vector<Component> _componentList{};

public:

	void Start()
	{

	}

	//constructor
	GameObject()
	{

	}
};

