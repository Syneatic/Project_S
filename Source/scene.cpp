/*
Author: Yan Chun
Co-Author: Harith, Jia Xi, Zachary Yee, Wei Jun
*/
#include "scene.hpp"
#include "renderer.hpp"
#include "physics.hpp"
#include "audio.hpp"
#include"particle.hpp"
#include "scene_debugger.hpp"
#include "gameobject.hpp"
#include "eventhandler.hpp"
#include "level_transition.hpp"

#include "components.hpp"
#include "save_game.hpp"

#include "profiler_ui.h"

float accumulator{ 0 };
ProfilerUI profilerUI;

void BuildDockSpace()
{
	const ImGuiViewport* vp = ImGui::GetMainViewport();
	if (vp->WorkSize.x <= 0.f || vp->WorkSize.y <= 0.f) return;

	ImGuiWindowFlags host_flags =
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

	ImGui::SetNextWindowPos(vp->WorkPos);
	ImGui::SetNextWindowSize(vp->WorkSize);
	ImGui::SetNextWindowViewport(vp->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	ImGui::Begin("##dockspace", nullptr, host_flags);
	ImGui::PopStyleVar(2);

	ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();
}

void Scene::InitializeGameObjects()
{
	Debug::Log("=== Initialize GameObjects ===");
	Debug::Log("Total GameObjects : ", _gameObjectList.size());
	for (auto& pgo : _gameObjectList)
	{
		pgo->UpdateWorldTransform();
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

	SaveGameManager::SaveData data;

	if (SaveGameManager::toLoad)
	{
		SaveGameManager::Load(_name, data);
		Debug::Log("Save file name: ", _name);
		if (auto* player = FindGameObjectByName("Player"))
		{
			player->transform().position = data.spawnPoint;

			if (auto* pc = player->GetComponent<PlayerController>())
			{
				pc->SetSpawnPoint(data.spawnPoint);
				EngineCTX::gameTimer = data.timer;
				Debug::Log("EngineCTX::gameTimer: ", EngineCTX::gameTimer, "\n");
			}
			player->UpdateWorldTransform();
			Debug::Log("Player Current Position Loaded");
		}
		SaveGameManager::toLoad = false;
	}

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

				if (go->active())
				{
					go->OnUpdate();
				}
				go->UpdateWorldTransform();
			}

			Audio::Update();
		}

		UISystem::Update();
		if (AEInputCheckTriggered(AEVK_ESCAPE) && SceneManager::ActiveScene()->cname() == "Play_Level")
		{
			if (!EngineCTX::isPaused)
				UISystem::TogglePauseMenuGame();	
		}

		accumulator += EngineCTX::dt;
		while (accumulator >= EngineCTX::fixedDt)
		{
			Physics::Step();
			accumulator -= EngineCTX::fixedDt;
		}

		Physics::SyncToLocal();

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

		BuildDockSpace();

		if (EngineCTX::debugMode)
		{
			profilerUI.Render();
		}
		Debugger::Tick(*this);

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

		if (auto* pc = player->GetComponent<PlayerController>())
		{
			data.spawnPoint = pc->GetSpawnPoint();
			data.timer = EngineCTX::gameTimer;
		}
		SaveGameManager::Save(data);
	}

	#ifdef _DEBUG
		Debugger::Reset();   
	#endif
	for (auto& pgo : _gameObjectList)
	{
		auto go = pgo.get();
		go->OnDestroy();
	}

	EngineCTX::gameTimer = 0.f;

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