#pragma once
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <string>
#include "component.hpp"

struct GameObject
{
private:
	std::string _name{};
	std::unordered_map<std::type_index, Component> _componentMap{};
	std::vector<GameObject> _children{};
	//std::vector<Component> _componentList{};

public:

	void Start()
	{

	}

	void AddComponent(Component c)
	{
		//_componentList.push_back(c);
	}



	//getters
	//std::vector<Component>& GetComponentList() { return _componentList; }
	std::string GetName() { return _name.empty() ? " " : _name; }
	//setters
	std::string SetName(std::string name) { return _name = name; }
	//constructor
	GameObject(char* name)
	{
		_name = name;
	}

	GameObject(std::string name)
	{
		_name = name;
	}

};

