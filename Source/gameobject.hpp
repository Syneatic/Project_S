#pragma once
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <string>
#include <memory>
#include <iostream>
#include "component.hpp"

struct GameObject
{
private:
	std::string _name{};
	std::unordered_map<std::type_index, std::unique_ptr<Component>> _componentMap{}; //only 1 of each type of component can be attached
	std::vector<std::unique_ptr<GameObject>> _children{};

public:

	void Start()
	{

	}

	template<class T>
	void AddComponent(const T& comp)
	{
		//tells the compiler to check if T derives from Component
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		std::type_index type{ typeid(T) };
		//check if key exists
		auto s = _componentMap.find(type);
		if (s != _componentMap.end())
		{
			std::cout << "Component already exists!" << std::endl;
			return;
		}

		auto ptr = std::make_unique<T>(comp);
		_componentMap.emplace(type,std::move(ptr));
	}

	//getters / setters
	std::unordered_map<std::type_index, std::unique_ptr<Component>>& componentMap() { return _componentMap; }
	std::string name() { return _name.empty() ? " " : _name; }
	std::string name(std::string name) { return _name = name; }

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

