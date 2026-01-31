#pragma once
#include <vector>
#include <string>

#include "gameobject.hpp"
#include "physics.hpp"
#include "renderer.hpp"
#include "render_components.hpp"

struct EditorScene;

struct Scene
{
protected:
	std::vector<std::unique_ptr<GameObject>> _gameObjectList{};
	std::string _name{};

	void InitializeGameObjects()
	{
		std::cout << "=== InitializeGameObjects ===" << std::endl;
		std::cout << "Total GameObjects: " << _gameObjectList.size() << std::endl;
		for (auto& pgo : _gameObjectList)
		{
			auto go = pgo.get();
			for (auto& [type, comp] : go->componentMap())
			{

				//register renderer
				if (auto* r = dynamic_cast<Renderer*>(comp.get()))
					RenderSystem::RegisterRenderer(r);

				//register collider
				if (auto* c = dynamic_cast<Collider*>(comp.get()))
				{
					std::cout << "  -> Registering Collider!" << std::endl;
					Physics::RegisterCollider(c);
				}
				//Register RigidBody
				if (auto* rb = dynamic_cast<RigidBody*>(comp.get()))
				{
					std::cout << "  -> Registering RigidBody!" << std::endl;
					Physics::RegisterRigidBody(rb);
				}
				//OnStart behaviours
				if (auto* b = dynamic_cast<Behaviour*>(comp.get()))
					b->OnStart();
				
				
			}
		}
	}

public:
	virtual void OnEnter()
	{
		//load data from file

		//initialize all gameobjects
		if (_gameObjectList.empty()) return;

		InitializeGameObjects();
	}

	virtual void OnUpdate()
	{
		//Physics::CheckAllTypeCollisions();
		//test draw
		AEGfxSetBackgroundColor(0.f,0.f,0.f);

		for (auto& pgo : _gameObjectList)
		{
			auto go = pgo.get();
			for (auto& [type, comp] : go->componentMap())
			{
				if (auto* b = dynamic_cast<Behaviour*>(comp.get()))
					b->OnUpdate();
			}
		}
		RenderSystem::Draw();
	}

	virtual void OnExit()
	{
		//delete
		RenderSystem::FlushRenderers();
		Physics::FlushColliders();
		Physics::FlushRigidBody();
	}

	//===== SERIALIZATION =====
	const std::string& name() const { return _name; }
	const std::string& name(std::string name) { return _name = std::move(name); }
	std::vector<std::unique_ptr<GameObject>>& gameObjectList() { return _gameObjectList; }
	const std::vector<std::unique_ptr<GameObject>>& gameObjectList() const { return _gameObjectList; }


	Scene() {}
	Scene(std::string name) { _name = std::move(name); }
	virtual ~Scene() = default;
};