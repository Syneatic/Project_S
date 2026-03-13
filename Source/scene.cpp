#include "scene.hpp"
#include "renderer.hpp"
#include "physics.hpp"
#include "audio.hpp"
#include"particle.hpp"

#include "gameobject.hpp"
#include "eventhandler.hpp"
#include "level_transition.hpp"

#include "components.hpp"

#include "profiler_ui.h"

float accumulator{ 0 };
ProfilerUI profilerUI;

void BuildDockSpace()
{
	ImGuiWindowFlags host_flags =
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

	const ImGuiViewport* vp = ImGui::GetMainViewport();
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
				go->UpdateWorldTransform();

				if (!go->active()) continue;

				go->OnUpdate();
			}

			Audio::Update();
		}

		if (AEInputCheckTriggered(AEVK_P))
			UISystem::TogglePauseMenuGame();

		if (AEInputCheckTriggered(AEVK_Z))
			UISystem::TempEndScreenPlsRemove();

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
	//Profiler::Get().SetPaused(EngineCTX::debugMode);
	if (EngineCTX::imguiInitialize)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		BuildDockSpace();
		if(EngineCTX::debugMode)
			profilerUI.Render();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGui::EndFrame();
	}
#endif
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
