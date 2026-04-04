/*
Author: Yan Chun
Co-Author: NIL
*/

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
	std::vector<std::type_index> _componentOrder{}; // tracks draw order

	GameObject* _parent{ nullptr };
	std::vector<std::unique_ptr<GameObject>> _children{}; //unused for now
	std::vector<EventHandler::SubscriptionHandle> _eventList{};
	Transform _transform{}; //this will always be local
	Transform _worldTransform{}; //always use this for physics/render

	void WorldToLocal()
	{
		if (!_parent) //local = world if no parent
		{
			_transform = _worldTransform;
			return;
		}

		const Transform& pw = _parent->_worldTransform;

		float2 worldPos = _worldTransform.position;

		//convert local to world
		float2 pos = (pw.scale != 0.f) ? (worldPos - pw.position) / pw.scale : (worldPos - pw.position);
		float2 scl = (pw.scale != 0.f)? _worldTransform.scale / pw.scale : _worldTransform.scale;
		float rot = _worldTransform.rotation - pw.rotation;

		_transform.position = pos;
		_transform.scale = scl;
		_transform.rotation = rot;
	}

	void LocalToWorld()
	{
		if (!_parent)
		{
			_worldTransform = _transform;
			return;
		}

		const Transform& pw = _parent->_worldTransform;
		float2 localPos = _transform.position;
		

		float2 pos = pw.position + (localPos * pw.scale);
		float2 scl = pw.scale * _transform.scale;
		float rot = pw.rotation + _transform.rotation;

		_worldTransform.position = pos;
		_worldTransform.scale = scl;
		_worldTransform.rotation = rot;

		//propogate to children as well
		for (auto& child : _children)
			child->LocalToWorld();
	}


public:
	void OnStart()
	{
		for (auto& [type, comp] : _componentMap)
			comp.get()->OnStart();
		
		for (auto& child : _children)
			child->OnStart();
	}

	void OnUpdate()
	{
		PROFILE_SCOPE(__func__);

		if (_active) //only update components if active
		{
			PROFILE_SCOPE("Components");
			for (auto& [type, comp] : _componentMap)
			{
				if (comp.get()->active())
				{
					PROFILE_SCOPE("OnUpdate");
					comp.get()->OnUpdate();
				}
			}		
		}

		for (auto& child : _children)
		{
			if (child->active())
			{
				child->OnUpdate();
			}
		}
	}

	void OnDestroy()
	{
		for (auto& [type, comp] : _componentMap)
			comp.get()->OnDestroy();

		for (auto& child : _children)
			child->OnDestroy();
	}

	// ===== COMPONENT =====
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
			GetOrAddComponent<AudioEmitter>();


		//check if alrdy exists
		std::type_index type{ typeid(T) };
		if (_componentMap.find(type) != _componentMap.end())
			return *dynamic_cast<T*>(_componentMap[type].get());

		auto ptr = std::make_unique<T>(*this);

		T& ref = *ptr;
		_componentMap.emplace(type, std::move(ptr));
		_componentOrder.push_back(type); // track order
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
		_componentOrder.erase(
			std::remove(_componentOrder.begin(), _componentOrder.end(), type),
			_componentOrder.end()
		);
	}

	template <typename T>
	bool HasComponent() const
	{
		static_assert(std::is_base_of<Component, T>::value, "T must derive from Component");
		return _componentMap.find(std::type_index(typeid(T))) != _componentMap.end();
	}

	// ===== COMPONENT =====

	// ===== HIERARCHY =====

	void AddChild(std::unique_ptr<GameObject> child)
	{
		child->_parent = this;
		_children.emplace_back(std::move(child));

		// Propagate so the child's _worldTransform is consistent immediately.
		_children.back()->UpdateWorldTransform(&_worldTransform);
	}

	//unparenting child
	std::unique_ptr<GameObject> RemoveChild(GameObject* child)
	{
		//find child
		auto it = std::find_if(_children.begin(), _children.end(),
			[child](const std::unique_ptr<GameObject>& c) { return c.get() == child; });

		if (it == _children.end()) return nullptr;

		//move child out
		std::unique_ptr<GameObject> owned = std::move(*it);
		_children.erase(it);

		owned->UpdateWorldTransform(&_worldTransform);

		// restore world transform as local so it stays in place after unparenting
		owned->_transform = owned->_worldTransform;
		owned->_parent = nullptr;
		owned->UpdateWorldTransform();
		return owned;
	}

	bool IsDescendantOf(const GameObject* ancestor) const
	{
		const GameObject* current = _parent;
		while (current)
		{
			if (current == ancestor) return true;
			current = current->_parent;
		}
		return false;
	}

	// ===== HIERARCHY =====

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

	//call on root objects only
	void UpdateWorldTransform(const Transform* parentWorld = nullptr)
	{
		if (parentWorld)
		{
			// combine parent world with local
			_worldTransform.position.x = parentWorld->position.x +
				(_transform.position.x * parentWorld->scale.x);
			_worldTransform.position.y = parentWorld->position.y +
				(_transform.position.y * parentWorld->scale.y);

			_worldTransform.scale.x = parentWorld->scale.x * _transform.scale.x;
			_worldTransform.scale.y = parentWorld->scale.y * _transform.scale.y;

			_worldTransform.rotation = parentWorld->rotation + _transform.rotation;
		}
		else
		{
			// root object - world == local
			_worldTransform =_transform;
		}

		// propagate to children
		for (auto& child : _children)
		{
			child->UpdateWorldTransform(&_worldTransform);
		}
	}


	//getters / setters
	ComponentMap&					componentMap() { return _componentMap; }
	const ComponentMap&				componentMap() const { return _componentMap; }
	std::vector<std::type_index>&	componentOrder() { return _componentOrder; }

	std::vector<std::unique_ptr<GameObject>>&		children() { return _children; }
	const std::vector<std::unique_ptr<GameObject>>& children() const { return _children; }
	GameObject* parent() const { return _parent; }

	std::string& name() { return _name; }
	const std::string& cname() const { return _name; }
	std::string& name(std::string name) { return _name = std::move(name); }

	const bool active() const { return _active; }
	bool active(bool state) { return _active = state; }

	const Transform& transform() const { return _transform; }
	Transform& transform() { return _transform; }

	const Transform& worldTransform() const { return _worldTransform; }
	Transform& worldTransform() { return _worldTransform; }

	//constructor
	GameObject(const char* name) : id(nextId++)
	{
		_name = name ? name : "";
	}

	GameObject(std::string name) : id(nextId++)
	{
		_name = name;
	}
	
//static func
	static std::unique_ptr<GameObject> Clone(const GameObject& src)
	{
		auto go = std::make_unique<GameObject>(src.cname() + "_clone");
		
		go->_transform = src._transform;
		go->_active = src._active;

		//copy components
		for (const auto& type : src._componentOrder)
		{
			auto it = src._componentMap.find(type);
			if (it == src._componentMap.end()) continue;

			auto cloned = it->second->Clone(*go);          // virtual dispatch
			go->_componentMap.emplace(type, std::move(cloned));
			go->_componentOrder.push_back(type);           // preserve draw order
		}

		for (const auto& child : src._children)
		{
			auto clonedChild = Clone(*child);              // recurse
			clonedChild->_parent = go.get();
			go->_children.emplace_back(std::move(clonedChild));
		}

		return go;
	}
};