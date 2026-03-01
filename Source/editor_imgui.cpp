//systems
#include "physics.hpp"
#include "gameobject.hpp"
#include "audio.hpp"
#include "camera.hpp"

//scene
#include "scene_parser.hpp"
#include "imgui_helper.hpp"
#include "editor_imgui.hpp"

//comps
#include "components.hpp"


namespace //helpers
{
	u32 selectedGameObjectIndex{};


	//void BuildDockSpace()
	//{
	//	ImGuiWindowFlags host_flags =
	//		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
	//		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
	//		ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
	//		ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

	//	const ImGuiViewport* vp = ImGui::GetMainViewport();
	//	ImGui::SetNextWindowPos(vp->WorkPos);
	//	ImGui::SetNextWindowSize(vp->WorkSize);
	//	ImGui::SetNextWindowViewport(vp->ID);

	//	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	//	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	//	ImGui::Begin("DockHost", nullptr, host_flags);
	//	ImGui::PopStyleVar(2);

	//	ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
	//	ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
	//	ImGui::End();
	//}

	//void BuildMenuBar()
	//{
	//	ImGui::BeginMainMenuBar();

	//	if (ImGui::BeginMenu("File"))
	//	{
	//		if (ImGui::MenuItem("Save"))
	//		{
	//			SceneIO::SerializeScene(loadedScene);
	//		}

	//		if (ImGui::MenuItem("Load"))
	//		{
	//			std::wstring fileW = OpenFile();
	//			if (!fileW.empty())
	//			{
	//				std::filesystem::path p(fileW);
	//				std::string fileNameNoExt = p.stem().string();
	//				SceneIO::DeserializeScene(loadedScene, fileNameNoExt);
	//				RefreshScene();
	//			}
	//		}

	//		ImGui::Separator();
	//		if (ImGui::MenuItem("Quit"))
	//		{
	//			//quit the application?
	//			//or return
	//		}

	//		ImGui::EndMenu();
	//	}

	//	ImGui::EndMainMenuBar();
	//}

	void BuildSceneHierarchyWindow(Scene& scene)
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(320.f, 100.f), ImVec2(FLT_MAX, FLT_MAX));

		ImGui::Begin("Scene");

		NameInputText(scene.name());

		//SelectableList();

		//iterate through scene objects and display them here
		for (int i = 0; i < scene.gameObjectList().size(); i++)
		{
			GameObject& gobj = *scene.gameObjectList()[i];
			bool isSelected = (selectedGameObjectIndex == i);

			ImGui::Selectable(gobj.name().c_str(), isSelected);

			//drag and drop
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("GO_REORDER", &i, sizeof(int));
				ImGui::TextUnformatted(gobj.name().c_str()); // tooltip while dragging
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GO_REORDER"))
				{
					int fromIndex = *(const int*)payload->Data;
					if (fromIndex != i)
					{
						// grab the object being dragged
						auto dragged = std::move(scene.gameObjectList()[fromIndex]);
						scene.gameObjectList().erase(scene.gameObjectList().begin() + fromIndex);
						scene.gameObjectList().insert(scene.gameObjectList().begin() + i, std::move(dragged));

						// fix up selected index to follow the moved object
						selectedGameObjectIndex = i;
					}
				}
				ImGui::EndDragDropTarget();
			}

			if (ImGui::IsItemClicked())
				selectedGameObjectIndex = i;
		}

		if (ImGui::BeginPopupContextWindow("SceneRightClickMenu"))
		{
			if (ImGui::MenuItem("Create GameObject"))
			{
				int index = (int)scene.gameObjectList().size();
				std::string name = "GameObject_" + std::to_string(index);
				scene.gameObjectList().push_back(std::make_unique<GameObject>(name));
				selectedGameObjectIndex = index;
			}

			if (selectedGameObjectIndex >= 0)
			{
				if (ImGui::MenuItem("Delete GameObject"))
				{
					scene.gameObjectList().erase(scene.gameObjectList().begin() + selectedGameObjectIndex);
					selectedGameObjectIndex = -1;
				}
			}
			ImGui::EndPopup();
		}

		ImGui::End();
	}

	void BuildInspectorWindow(Scene& scene)
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(320.f, 100.f), ImVec2(FLT_MAX, FLT_MAX));
		ImGui::Begin("Inspector");

		//check if an object is selected
		if (selectedGameObjectIndex < 0) { ImGui::End(); return; }

		//display selected object's properties
		//iterate through each component and display its properties here
		//name text box
		GameObject& selectedObj = *scene.gameObjectList()[selectedGameObjectIndex];
		auto& transform = selectedObj.transform();

		NameInputText(selectedObj.name());

		//draw transform
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Float2DragReset("Position", &transform.position.x, {0.f,0.f}, 0.05f);
			Float2DragReset("Scale", &transform.scale.x, { 1.f,1.f }, 0.05f);
			FloatDragReset("Rotation", &transform.rotation, 0.f, 0.1f);
			ImGui::Separator();
		}

		//drawing of component elements
		const auto& comps = selectedObj.componentMap();
		for (auto it = comps.begin(); it != comps.end(); ++it)
		{
			const std::type_index& type = it->first;
			const std::unique_ptr<Component>& compPtr = it->second;

			if (!compPtr) continue;

			bool open = ImGui::CollapsingHeader(compPtr.get()->name().c_str(), ImGuiTreeNodeFlags_DefaultOpen);

			if (ImGui::BeginPopupContextItem())
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					selectedObj.RemoveComponent(type);
					ImGui::EndPopup();
					break;
				}
				ImGui::EndPopup();
			}

			if (open)
			{
				compPtr.get()->DrawInInspector();
				ImGui::Separator();
			}
		}

		ImGui::Separator();

		//COMPONENTS

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponentMenu");
		}

		if (ImGui::BeginPopup("AddComponentMenu"))
		{
			ComponentSubMenu("Physics", { "Box Collider","Circle Collider","Rigid Body"},
				[&](int i)
				{
					if (i == 0) selectedObj.AddComponent<BoxCollider>();
					if (i == 1) selectedObj.AddComponent<CircleCollider>();
					if (i == 2) selectedObj.AddComponent<RigidBody>();
				});

			ComponentSubMenu("Renderer", { "Sprite Renderer","Mesh Renderer" },
				[&](int i)
				{
					if (i == 0) selectedObj.AddComponent<SpriteRenderer>();
					if (i == 1) selectedObj.AddComponent<MeshRenderer>();
				});

			if (ImGui::BeginMenu("Particle"))
			{
				if (ImGui::MenuItem("Particle Emitter"))
				{
					selectedObj.AddComponent<ParticleEmitter>();
				}

				ImGui::EndMenu();
			}

			ComponentSubMenu("Audio", { "Audio Emitter","Audio Listener" },
				[&](int i)
				{
					if (i == 0) selectedObj.AddComponent<AudioEmitter>();
					if (i == 1) selectedObj.AddComponent<AudioListener>();
				});

			ComponentSubMenu("UI", { "Display", "Button", "Text"},
				[&](int i)
				{
					if (i == 0) selectedObj.AddComponent<Display>();
					if (i == 1) selectedObj.AddComponent<Button>();
					if (i == 2) selectedObj.AddComponent<TextRenderer>();
				});
			

			if (ImGui::MenuItem("Main Camera"))
			{
				selectedObj.AddComponent<MainCamera>();
			}

			ImGui::Separator();
			ImGui::TextUnformatted("Custom");
			ImGui::Separator();
			//we put our own components here
			ComponentSubMenu("Controller", { "Player Controller","Rock Controller" },
				[&](int i)
				{
					if (i == 0) selectedObj.AddComponent<PlayerController>();
					if (i == 1) selectedObj.AddComponent<RockController>();
				});

			if (ImGui::MenuItem("Noise Source"))
			{
				selectedObj.AddComponent<NoiseSource>();
			}


			ImGui::EndPopup();
		}

		ImGui::End();
	}
}

namespace Editor
{
	void DrawUI(Scene& scene)
	{
		//BuildDockSpace();
		//BuildMenuBar();
		//BuildSceneHierarchyWindow();
		BuildInspectorWindow(scene);
	}
}
