#include "scene.hpp"
//#include "imgui"
#include "ImGUI/imgui.h"

struct LevelEditorScene : Scene
{
	void DrawImGui()
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(320.f, 100.f), ImVec2(FLT_MAX, FLT_MAX));
		ImGui::Begin("Inspector");
		ImGui::Text("Bang");

		ImGui::End();
	}


	void InitializeScene() override
	{
		//read from file
		//initialize all gameobjects
		/*
		* for each game objects
		*	=> gameobjects.Start()
		*/
	}

	void UpdateScene() override
	{

	}

	void DestroyScene() override
	{
		//delete
	}
};