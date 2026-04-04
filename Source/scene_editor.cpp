/*
Author: Yan Chun
Co-Author: Nil
*/
//systems
#include "renderer.hpp"
#include "physics.hpp"
#include "gameobject.hpp"
#include "audio.hpp"
#include "camera.hpp"
#include "particle.hpp"

//scene
#include "scene_parser.hpp"
#include "scene_editor.hpp"
#include "editor.hpp"

//comps
#include "components.hpp"

namespace
{
	void ReadInput(EditorScene& /*escene*/, Scene& scene)
	{
		if (AEInputCheckCurr(AEVK_LCTRL) && AEInputCheckTriggered(AEVK_S))
		{
			Editor::SaveScene(scene);
			Debug::Log("Scene saved!");
		}

		if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
		{
			//if any imgui window not focused
			CameraSystem::ZoomInput();
		}
	}

	void UpdateGO(GameObject& go)
	{
		if (!(go.active())) return;

		for (auto& [type, comp] : go.componentMap())
		{
			if (auto* c = dynamic_cast<Renderer*>(comp.get()))
			{
				if(c->active())
					c->OnUpdate();
			}

			if (auto* c = dynamic_cast<ParticleEmitter*>(comp.get()))
			{
				if (c->active())
					c->OnUpdate();
			}
		}

		if (!go.children().empty())
		{
			for (auto& child : go.children())
			{
				 UpdateGO(*child);
			}
		}
	}
}

namespace Editor
{
	void SaveScene(Scene& scene)
	{
		SceneIO::SerializeScene(scene);
	}
}

void EditorScene::RefreshScene()
{
	Editor::selectedObjects.clear(); //reset index selection
	CameraSystem::OnStart();
}

void EditorScene::OnEnter()
{
	//start on prototype for testing
	SceneIO::DeserializeScene(loadedScene, "Play_Level");
	RefreshScene();
	ParticleSystem::Initialize();
	CameraSystem::OnStart();
}

void EditorScene::OnUpdate()
{
	ReadInput(*this,loadedScene);
	CameraSystem::OnUpdate(); // Check input and update camera matrix

	AEGfxSetBackgroundColor(0.f, 0.f, 0.f);
	//bool imguiFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);

	//draw gizmos last
	Editor::DrawGizmos();

	//uniquely for editor only
	for (auto& pgo : loadedScene.gameObjectList())
	{
		auto* go = pgo.get();
		go->UpdateWorldTransform();
		UpdateGO(*go);
	}

	ParticleSystem::Update();
	Graphics::Execute();
	//we shall simulate particles too
	ParticleSystem::Render();

	//draw imgui after game render
	if (EngineCTX::imguiInitialize)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		Editor::DrawUI(*this,loadedScene);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		ImGui::EndFrame();
	}
}

void EditorScene::OnExit() 
{
	//unload everything
	Graphics::Flush();
	Physics::Flush();
	CameraSystem::OnExit();
}
