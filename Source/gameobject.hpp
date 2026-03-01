#pragma once

#include "components.hpp"
#include "eventhandler.hpp"

class GameObject
{
public:
	using ComponentMap = std::unordered_map<std::type_index, std::unique_ptr<Component>>;

	inline static size_t nextId{ 0 };
	const size_t id;
private:
	bool _active{ true };
	std::string _name{};
	ComponentMap _componentMap{}; //only 1 of each type of component can be attached
	std::vector<std::unique_ptr<GameObject>> _children{}; //unused for now
	std::vector<EventHandler::SubscriptionHandle> _eventList{};
	Transform _transform{};

public:
	void Start()
	{
		for (auto& [type, comp] : _componentMap)
		{
			comp.get()->OnStart();
		}
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

	template<class T>
	T& AddComponent()
	{
		//tells the compiler to check if T derives from Component
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		if constexpr (std::is_base_of<NoiseSource, T>::value)
		{
			GetOrAddComponent<AudioEmitter>();
		}


		std::type_index type{ typeid(T) };
		//check if alrdy exists
		if (_componentMap.find(type) != _componentMap.end())
		{
			T* component = dynamic_cast<T*>(_componentMap[type].get());
			return *component;
		}

		auto ptr = std::make_unique<T>(*this);

		T& ref = *ptr;
		_componentMap.emplace(type, std::move(ptr));
		return ref;
	}

	template<class T>
	T& GetOrAddComponent()
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");

		if (auto* existing = GetComponent<T>())
			return *existing;

		return AddComponent<T>();
	}

	void RemoveComponent(std::type_index type)
	{
		_componentMap.erase(type);
	}

	void AddChild(std::unique_ptr<GameObject> child)
	{
		_children.emplace_back(std::move(child));
	}

	template <typename T, typename M>	
	void Subscribe(M T::* member, M matchValue, std::function<void(const T&)> func)
	{
		auto handle = EventHandler::SubscribeFilter<T, M>
			(
			member,
			matchValue,
			func
		);

		_eventList.push_back(handle);
	}

	void Unsubscribe(const EventHandler::SubscriptionHandle& handle)
	{
		EventHandler::Unsubscribe(handle);

		_eventList.erase(
			//find the handle
			std::remove_if(_eventList.begin(), _eventList.end(),
				[&](const EventHandler::SubscriptionHandle& h) {
					return h.id == handle.id && h.type == handle.type;
				}),
			_eventList.end()
		);
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

	std::string& name() { return _name; }
	const std::string& cname() const { return _name; }
	std::string& name(std::string name) { return _name = std::move(name); }

	const bool active() const { return _active; }
	bool active(bool state) { return _active = state; }

	const Transform& transform() const { return _transform; }
	Transform& transform() { return _transform; }

	//constructor
	GameObject(const char* name) : id(nextId++)
	{
		_name = name ? name : "";
	}

	GameObject(std::string name) : id(nextId++)
	{
		_name = name;
	}
};

//for now dont use
//static GameObject* CreateGameObject(const char* name)
//{
//	return new GameObject(name);
//}
