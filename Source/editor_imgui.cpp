//systems
#include "physics.hpp"
#include "gameobject.hpp"
#include "audio.hpp"
#include "camera.hpp"

//scene
#include "scene_parser.hpp"
#include "scene_editor.hpp"
#include "imgui_helper.hpp"
#include "editor_imgui.hpp"

//comps
#include "components.hpp"


namespace //helpers
{
	std::wstring OpenFile()
	{
		//get current directory
		//wchar_t cwd[MAX_PATH]{};
		//GetCurrentDirectoryW(MAX_PATH, cwd);
		namespace fs = std::filesystem;
		std::wstring targetDir = L"../../Assets/Scene/";

		try {
			if (fs::exists(targetDir)) {
				targetDir = fs::absolute(targetDir).wstring();
			}
		}
		catch (...) {}

		//create dialog
		HRESULT hrInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		bool didCoInit = SUCCEEDED(hrInit) || hrInit == RPC_E_CHANGED_MODE;

		//check if successful
		IFileOpenDialog* pfd = nullptr;
		HRESULT hrCreate = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
		if (FAILED(hrCreate) || !pfd)
		{
			if (didCoInit && hrCreate != RPC_E_CHANGED_MODE) CoUninitialize();
			return L"";
		}

		//configure dialog
		pfd->SetTitle(L"Open Scene");

		// Optional: filter
		COMDLG_FILTERSPEC filters[] =
		{
			{ L"Scene files (*.scene)", L"*.scene" },
			{ L"All files (*.*)",       L"*.*"     }
		};
		pfd->SetFileTypes((UINT)std::size(filters), filters);
		pfd->SetFileTypeIndex(1);

		//starting folder
		IShellItem* startFolder = nullptr;
		if (SUCCEEDED(SHCreateItemFromParsingName(targetDir.c_str(), nullptr, IID_PPV_ARGS(&startFolder))))
		{
			pfd->SetFolder(startFolder);
			pfd->SetDefaultFolder(startFolder);
			startFolder->Release();
		}

		//display dialog
		HRESULT hrShow = pfd->Show(nullptr);

		std::wstring out;
		if (SUCCEEDED(hrShow))
		{
			IShellItem* result = nullptr;
			if (SUCCEEDED(pfd->GetResult(&result)) && result)
			{
				PWSTR path = nullptr;
				if (SUCCEEDED(result->GetDisplayName(SIGDN_FILESYSPATH, &path)) && path)
				{
					out = path;
					CoTaskMemFree(path);
				}
				result->Release();
			}
		}

		pfd->Release();
		if (didCoInit && hrInit != RPC_E_CHANGED_MODE)
			CoUninitialize();

		return out;
	}

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

	void BuildMenuBar(EditorScene& escene, Scene& scene)
	{
		ImGui::BeginMainMenuBar();

		if (ImGui::BeginMenu("File##mainmenu"))
		{
			if (ImGui::MenuItem("Save##mainmenu"))
			{
				SceneIO::SerializeScene(scene);
			}

			if (ImGui::MenuItem("Load##mainmenu"))
			{
				std::wstring fileW = OpenFile();
				if (!fileW.empty())
				{
					std::filesystem::path p(fileW);
					std::string fileNameNoExt = p.stem().string();
					SceneIO::DeserializeScene(scene, fileNameNoExt);
				}

				escene.RefreshScene();
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Quit##mainmenu"))
			{
				EngineCTX::applicationRunning = false;
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	void BuildSceneHierarchyWindow(Scene& scene)
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(320.f, 100.f), ImVec2(FLT_MAX, FLT_MAX));
		ImGui::Begin("Scene");

		NameInputText(scene.name());

		bool didDrag = false;

		for (int i = 0; i < (int)scene.gameObjectList().size(); i++)
		{
			GameObject& gobj = *scene.gameObjectList()[i];
			bool isSelected = std::find(Editor::selectedIndices.begin(), Editor::selectedIndices.end(), i)
				!= Editor::selectedIndices.end();

			ImGui::Selectable(gobj.name().c_str(), isSelected);

			// selection logic
			if (ImGui::IsItemClicked())
			{
				if (ImGui::GetIO().KeyCtrl)
				{
					auto it = std::find(Editor::selectedIndices.begin(), Editor::selectedIndices.end(), i);
					if (it != Editor::selectedIndices.end())
						Editor::selectedIndices.erase(it);
					else
						Editor::selectedIndices.push_back(i);
				}
				else if (ImGui::GetIO().KeyShift && !Editor::selectedIndices.empty())
				{
					int last = Editor::selectedIndices.back();
					int from = std::min(last, i), to = std::max(last, i);
					for (int j = from; j <= to; j++)
						if (std::find(Editor::selectedIndices.begin(), Editor::selectedIndices.end(), j)
							== Editor::selectedIndices.end())
							Editor::selectedIndices.push_back(j);
				}
				else
				{
					bool alreadySelected = std::find(Editor::selectedIndices.begin(),
						Editor::selectedIndices.end(), i) != Editor::selectedIndices.end();
					if (!alreadySelected)
						Editor::selectedIndices = { i };
				}
			}

			// collapse selection to single on release (only if no drag occurred)
			if (ImGui::IsItemDeactivated() && !ImGui::GetIO().KeyCtrl && !ImGui::GetIO().KeyShift)
			{
				bool alreadySelected = std::find(Editor::selectedIndices.begin(),
					Editor::selectedIndices.end(), i) != Editor::selectedIndices.end();
				if (alreadySelected && !ImGui::IsMouseDragging(0) && !didDrag)
					Editor::selectedIndices = { i };
			}

			// drag source
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				didDrag = true;

				bool partOfSelection = std::find(Editor::selectedIndices.begin(),
					Editor::selectedIndices.end(), i) != Editor::selectedIndices.end();
				if (!partOfSelection)
					Editor::selectedIndices = { i };

				ImGui::SetDragDropPayload("GO_REORDER", &i, sizeof(int));

				if (Editor::selectedIndices.size() > 1)
					ImGui::Text("%d objects", (int)Editor::selectedIndices.size());
				else
					ImGui::TextUnformatted(gobj.name().c_str());

				ImGui::EndDragDropSource();
			}

			// drop indicator line
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) && ImGui::GetDragDropPayload() != nullptr)
			{
				float itemMinY = ImGui::GetItemRectMin().y;
				float itemMaxY = ImGui::GetItemRectMax().y;
				bool insertAfter = ImGui::GetMousePos().y > (itemMinY + itemMaxY) * 0.5f;

				float lineY = insertAfter ? itemMaxY : itemMinY;
				float lineXMin = ImGui::GetItemRectMin().x;
				float lineXMax = ImGui::GetItemRectMax().x;

				ImDrawList* fg = ImGui::GetForegroundDrawList();
				fg->AddLine(
					ImVec2(lineXMin, lineY),
					ImVec2(lineXMax, lineY),
					IM_COL32(255, 80, 0, 255), 2.0f
				);
			}

			// drop target
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GO_REORDER"))
				{
					int fromIndex = *(const int*)payload->Data;

					float itemMinY = ImGui::GetItemRectMin().y;
					float itemMaxY = ImGui::GetItemRectMax().y;
					bool insertAfter = ImGui::GetMousePos().y > (itemMinY + itemMaxY) * 0.5f;
					int insertPos = insertAfter ? i + 1 : i;

					if (fromIndex != insertPos && fromIndex != insertPos - 1)
					{
						std::vector<int> sorted = Editor::selectedIndices;
						std::sort(sorted.begin(), sorted.end());

						std::vector<std::unique_ptr<GameObject>> dragged;
						for (int idx : sorted)
							dragged.push_back(std::move(scene.gameObjectList()[idx]));

						std::sort(sorted.begin(), sorted.end(), std::greater<int>());
						for (int idx : sorted)
							scene.gameObjectList().erase(scene.gameObjectList().begin() + idx);

						for (int idx : sorted)
							if (idx < insertPos) insertPos--;
						insertPos = std::clamp(insertPos, 0, (int)scene.gameObjectList().size());

						Editor::selectedIndices.clear();
						for (int j = 0; j < (int)dragged.size(); j++)
						{
							scene.gameObjectList().insert(scene.gameObjectList().begin() + insertPos + j, std::move(dragged[j]));
							Editor::selectedIndices.push_back(insertPos + j);
						}
					}
				}
				ImGui::EndDragDropTarget();
			}
		}

		if (ImGui::BeginPopupContextWindow("SceneRightClickMenu"))
		{
			if (ImGui::MenuItem("Create GameObject"))
			{
				int index = (int)scene.gameObjectList().size();
				std::string name = "GameObject_" + std::to_string(index);
				scene.gameObjectList().push_back(std::make_unique<GameObject>(name));
				Editor::selectedIndices.clear();
				Editor::selectedIndices.push_back(index);
			}

			if (!Editor::selectedIndices.empty())
			{
				if (ImGui::MenuItem("Delete GameObject"))
				{
					std::vector<int> sorted = Editor::selectedIndices;
					std::sort(sorted.begin(), sorted.end(), std::greater<int>());
					for (int idx : sorted)
						scene.gameObjectList().erase(scene.gameObjectList().begin() + idx);
					Editor::selectedIndices.clear();
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
		if (Editor::selectedIndices.empty()) { ImGui::End(); return; }
		GameObject& selectedObj = *scene.gameObjectList()[Editor::selectedIndices[0]];

		//display selected object's properties
		//iterate through each component and display its properties here
		//name text box
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
		auto& compOrder = selectedObj.componentOrder();
		for (int i = 0; i < (int)compOrder.size(); i++)
		{
			const std::type_index& type = compOrder[i];
			auto it = comps.find(type);
			if (it == comps.end() || !it->second) continue;

			const std::unique_ptr<Component>& compPtr = it->second;

			bool open = ImGui::CollapsingHeader(compPtr->name().c_str(), ImGuiTreeNodeFlags_DefaultOpen);

			// drag source
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
			{
				ImGui::SetDragDropPayload("COMP_REORDER", &i, sizeof(int));
				ImGui::TextUnformatted(compPtr->name().c_str());
				ImGui::EndDragDropSource();
			}

			// drop target
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("COMP_REORDER"))
				{
					int fromIndex = *(const int*)payload->Data;
					if (fromIndex != i)
					{
						auto dragged = compOrder[fromIndex];
						compOrder.erase(compOrder.begin() + fromIndex);
						compOrder.insert(compOrder.begin() + i, dragged);
					}
				}
				ImGui::EndDragDropTarget();
			}

			// right click remove
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
				compPtr->DrawInInspector();
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
			ComponentSubMenu("Controller", { "Player Controller","Rock Controller","Enemy Controller"},
				[&](int i)
				{
					if (i == 0) selectedObj.AddComponent<PlayerController>();
					if (i == 1) selectedObj.AddComponent<RockController>();
					if (i == 2) selectedObj.AddComponent<EnemyController>();
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
	void DrawUI(EditorScene& escene, Scene& scene)
	{
		BuildDockSpace();
		BuildMenuBar(escene,scene);
		BuildSceneHierarchyWindow(scene);
		BuildInspectorWindow(scene);
	}
}
