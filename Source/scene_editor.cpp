
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
	void UpdateGO(GameObject& go)
	{
		for (auto& [type, comp] : go.componentMap())
		{
			if (auto* c = dynamic_cast<Renderer*>(comp.get()))
			{
				c->OnUpdate();
			}

			if (auto* c = dynamic_cast<ParticleEmitter*>(comp.get()))
			{
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

void EditorScene::RefreshScene()
{
	Editor::selectedObjects.clear(); //reset index selection
	CameraSystem::OnStart();
}

void EditorScene::OnEnter()
{
	//start on prototype for testing
	SceneIO::DeserializeScene(loadedScene, "PrototypeLvl");


	RefreshScene();
	ParticleSystem::Initialize();
	CameraSystem::OnStart();
}

void EditorScene::OnUpdate()
{
	CameraSystem::OnUpdate(); // Check input and update camera matrix

	AEGfxSetBackgroundColor(0.f, 0.f, 0.f);
	//bool imguiFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow);

	//draw gizmos last
	Editor::DrawGizmos();
	//Gizmos(); //gizmos execution

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
	Physics::FlushColliders();
	Physics::FlushRigidBody();
	CameraSystem::OnExit();
}
