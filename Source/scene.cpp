#include "scene.hpp"
#include "renderer.hpp"
#include "physics.hpp"
#include "audio.hpp"
#include"particle.hpp"

#include "gameobject.hpp"
#include "eventhandler.hpp"
#include "level_transition.hpp"

#include "components.hpp"

float accumulator{ 0 };

void Scene::InitializeGameObjects()
{
	Debug::Log("=== Initialize GameObjects ===");
	Debug::Log("Total GameObjects : ", _gameObjectList.size());
	for (auto& pgo : _gameObjectList)
	{
		pgo.get()->OnStart();
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

	ParticleSystem::Initialize();
	ParticleSystem::Flush();
	InitializeGameObjects();

	Physics::Initialize();

	UISystem::init();
	LevelTransition::Init();
	CameraSystem::OnStart(); //reset camera
}

void Scene::OnUpdate()
{
	//test draw
	AEGfxSetBackgroundColor(0.f,0.f,0.f);
	LevelTransition::Update();

	for (auto& pgo : _gameObjectList)
	{
		auto go = pgo.get();
		if (!go->active()) continue;

		go->OnUpdate();
	}

	Audio::Update();

	if (AEInputCheckTriggered(AEVK_P))
		EngineCTX::PauseTime();

	accumulator += EngineCTX::dt;
	while (accumulator >= EngineCTX::fixedDt)
	{
		Physics::Step();
		accumulator -= EngineCTX::fixedDt;
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
		go->OnDestroy();
	}

	//delete
	EventHandler::Flush();
	Physics::Flush();
	Audio::FlushEmitters();
	ParticleSystem::Flush();
	Graphics::Flush();
}

//===== SERIALIZATION =====
std::string& Scene::name()  { return _name; }
const std::string& Scene::cname() const { return _name; }
const std::string& Scene::name(std::string name) { return _name = std::move(name); }
std::vector<std::unique_ptr<GameObject>>& Scene::gameObjectList() { return _gameObjectList; }
const std::vector<std::unique_ptr<GameObject>>& Scene::gameObjectList() const { return _gameObjectList; }

//con/destructors
Scene::Scene() {}
Scene::Scene(std::string name) { _name = std::move(name); }
Scene::~Scene() = default;
