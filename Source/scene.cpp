#include "scene.hpp"
#include "renderer.hpp"
#include "physics.hpp"
#include "audio.hpp"
#include"particle.hpp"

#include "gameobject.hpp"
#include "eventhandler.hpp"

#include "components.hpp"

float accumulator{ 0 };
float fixedDt = 1.f / 60.f;

void Scene::InitializeGameObjects()
{
	Debug::Log("=== Initialize GameObjects ===");
	Debug::Log("Total GameObjects", _gameObjectList.size());
	for (auto& pgo : _gameObjectList)
	{
		pgo.get()->Start();
	}
}

GameObject* Scene::FindGameObjectByName(const std::string& name)
{
    for (auto& go : _gameObjectList)
    {
        if (go->name() == name)
            return go.get();
    }
    return nullptr;
}

void Scene::OnEnter()
{
	//load data from file

	//initialize all gameobjects
	if (_gameObjectList.empty()) return;

	InitializeGameObjects();

	UISystem::init();
	ParticleSystem::Initialize();
	CameraSystem::OnStart(); //reset camera
}

void Scene::OnUpdate()
{
	f32 dt = (f32)AEFrameRateControllerGetFrameTime();
	//test draw
	AEGfxSetBackgroundColor(0.f,0.f,0.f);

	for (auto& pgo : _gameObjectList)
	{
		auto go = pgo.get();
		for (auto& [type, comp] : go->componentMap())
		{
			comp.get()->OnUpdate();
		}
	}

	Audio::Update();

	accumulator += dt;
	while (accumulator >= fixedDt)
	{
		Physics::Step(fixedDt);
		accumulator -= fixedDt;
	}

	ParticleSystem::Update();
	EventHandler::CallQ();
	Graphics::Execute();
	ParticleSystem::Render();
}

void Scene::OnExit()
{
	for (auto& pgo : _gameObjectList)
	{
		auto go = pgo.get();
		for (auto& [type, comp] : go->componentMap())
		{
			comp.get()->OnDestroy();
		}
	}

	//delete
	EventHandler::Flush();
	Physics::FlushColliders();
	Physics::FlushRigidBody();
	Audio::FlushEmitters();
	ParticleSystem::Flush();
	Graphics::Flush();
}

//===== SERIALIZATION =====
const std::string& Scene::name() const { return _name; }
const std::string& Scene::name(std::string name) { return _name = std::move(name); }
std::vector<std::unique_ptr<GameObject>>& Scene::gameObjectList() { return _gameObjectList; }
const std::vector<std::unique_ptr<GameObject>>& Scene::gameObjectList() const { return _gameObjectList; }

//con/destructors
Scene::Scene() {}
Scene::Scene(std::string name) { _name = std::move(name); }
Scene::~Scene() = default;
