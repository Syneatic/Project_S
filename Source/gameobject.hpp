#pragma once
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <string>

#include "component.hpp"

struct GameObject
{
	using ComponentMap = std::unordered_map<std::type_index, std::unique_ptr<Component>>;

private:
	bool _active{ true };
	std::string _name{};
	ComponentMap _componentMap{}; //only 1 of each type of component can be attached
	std::vector<std::unique_ptr<GameObject>> _children{};

	void Initialize()
	{
		
	}

public:
	void Start()
	{
		
	}

	template<class T, class... Args>
	T& AddComponent(Args&&... args)
	{
		//tells the compiler to check if T derives from Component
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		std::type_index type{ typeid(T) };
		//check if alrdy exists
		if (_componentMap.find(type) != _componentMap.end())
			throw std::runtime_error("Component already exists");

		auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
		ptr->SetOwner(this);

		T& ref = *ptr;
		_componentMap.emplace(type, std::move(ptr));
		return ref;
	}

	void RemoveComponent(std::type_index type)
	{
		_componentMap.erase(type);
	}

	void AddChild(std::unique_ptr<GameObject> child)
	{
		_children.emplace_back(std::move(child));
	}

	template<typename T>
	T* GetComponent()
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		auto it = _componentMap.find(std::type_index(typeid(T)));
		if (it == _componentMap.end())
			return nullptr;

		return static_cast<T*>(it->second.get());
	}

	template <typename T>
	const T* GetComponent() const
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		auto it = _componentMap.find(std::type_index(typeid(T)));
		if (it == _componentMap.end())
			return nullptr;

		return static_cast<const T*>(it->second.get());
	}

	template <typename T>
	bool HasComponent() const
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
		return _componentMap.find(std::type_index(typeid(T))) != _componentMap.end();
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

