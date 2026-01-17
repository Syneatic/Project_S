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
	using ComponentMap = std::unordered_map<std::type_index, std::unique_ptr<Component>>;

private:
	bool _active{ true };
	std::string _name{};
	ComponentMap _componentMap{}; //only 1 of each type of component can be attached
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

	void RemoveComponent(std::type_index type)
	{
		_componentMap.erase(type);
	}

	void AddChild(std::unique_ptr<GameObject> child)
	{
		_children.emplace_back(std::move(child));
	}

	//getters / setters
	ComponentMap& componentMap() { return _componentMap; }
	const ComponentMap& componentMap() const { return _componentMap; }

	std::vector<std::unique_ptr<GameObject>>& children() { return _children; }
	const std::vector<std::unique_ptr<GameObject>>& children() const { return _children; }

	const std::string name() const { return _name.empty() ? " " : _name; }
	std::string name(std::string name) { return _name = std::move(name); }

	const bool active() const { return _active; }
	bool active(bool state) { return _active = state; }

	//constructor
	GameObject(const char* name)
	{
		_name = name ? name : "";
	}

	GameObject(std::string name)
	{
		_name = name;
	}

};

