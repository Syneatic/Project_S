#include "scene.hpp"
#include "renderer.hpp"
#include "physics.hpp"
#include "audio.hpp"
#include"particle.hpp"

#include "gameobject.hpp"
#include "eventhandler.hpp"
#include "level_transition.hpp"

#include "components.hpp"
#include "save_game.hpp"

#include "profiler_ui.h"

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

		for (auto& goC : go->children())
		{
			if (goC->name() == name)
				return goC.get();
		}
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

	SaveGameManager::SaveData data;

	if (SaveGameManager::Load(_name, data))
	{
		Debug::Log("Save file name: ", _name);
		if (auto* player = FindGameObjectByName("Player"))
		{
			player->transform().position = data.playerPosition;

			if (auto* pc = player->GetComponent<PlayerController>())
			{
				pc->SetSpawnPoint(data.spawnPoint);
			}

			Debug::Log("Player Current Position Loaded");
		}
	}

	UISystem::init();
	LevelTransition::Init();
	CameraSystem::OnStart(); //reset camera
}

void Scene::OnUpdate()
{
	ProfilerUI profilerUI;

	PROFILE_FRAME_BEGIN();
	{
		PROFILE_SCOPE("GameLoop");

		//test draw
		AEGfxSetBackgroundColor(0.f,0.f,0.f);
		LevelTransition::Update();

		{
			PROFILE_SCOPE("Update GameObjects");
			for (auto& pgo : _gameObjectList)
			{
				auto go = pgo.get();
				if (!go->active()) continue;

				go->OnUpdate();
			}

			Audio::Update();
		}


		if (AEInputCheckTriggered(AEVK_P))
			EngineCTX::PauseTime();

		accumulator += EngineCTX::dt;
		while (accumulator >= EngineCTX::fixedDt)
		{
			Physics::Step();
			accumulator -= EngineCTX::fixedDt;
		}

		{
			PROFILE_SCOPE("Particle");
			ParticleSystem::Update();
			ParticleSystem::Render();
		}
		EventHandler::CallQ();
		Graphics::Execute();
	}
	PROFILE_FRAME_END();


#ifdef _DEBUG
	if (EngineCTX::imguiInitialize)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		profilerUI.Render();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGui::EndFrame();
	}
#endif
}

void Scene::OnExit()
{
	if (auto* player = FindGameObjectByName("Player"))
	{
		SaveGameManager::SaveData data;
		data.sceneName = _name;
		data.playerPosition = player->transform().position;

		if (auto* pc = player->GetComponent<PlayerController>())
		{
			data.spawnPoint = pc->GetSpawnPoint();
		}
		SaveGameManager::Save(data);
	}

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
