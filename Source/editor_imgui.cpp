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

	enum class DropZone { None, Above, Into, Below };

	DropZone GetDropZone()
	{
		float minY = ImGui::GetItemRectMin().y;
		float maxY = ImGui::GetItemRectMax().y;
		float height = maxY - minY;
		float mouseY = ImGui::GetMousePos().y;
		float relY = mouseY - minY;

		if (relY < height * 0.25f)      return DropZone::Above;
		else if (relY > height * 0.75f) return DropZone::Below;
		else                            return DropZone::Into;
	}

	void DrawDropIndicator(DropZone zone)
	{
		float minX = ImGui::GetItemRectMin().x;
		float maxX = ImGui::GetItemRectMax().x;
		float minY = ImGui::GetItemRectMin().y;
		float maxY = ImGui::GetItemRectMax().y;
		ImDrawList* fg = ImGui::GetForegroundDrawList();

		if (zone == DropZone::Above)
		{
			fg->AddLine(ImVec2(minX, minY), ImVec2(maxX, minY), IM_COL32(255, 80, 0, 255), 2.f);
			fg->AddCircleFilled(ImVec2(minX, minY), 3.f, IM_COL32(255, 80, 0, 255));
		}
		else if (zone == DropZone::Below)
		{
			fg->AddLine(ImVec2(minX, maxY), ImVec2(maxX, maxY), IM_COL32(255, 80, 0, 255), 2.f);
			fg->AddCircleFilled(ImVec2(minX, maxY), 3.f, IM_COL32(255, 80, 0, 255));
		}
		else if (zone == DropZone::Into)
		{
			// highlight the whole item in blue like Unity
			fg->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY), IM_COL32(50, 150, 255, 255), 0.f, 0, 2.f);
		}
	}

	std::unique_ptr<GameObject> ExtractFromScene(Scene& scene, GameObject* target)
	{
		// search root list first
		auto& roots = scene.gameObjectList();
		for (auto it = roots.begin(); it != roots.end(); ++it)
		{
			if (it->get() == target)
			{
				auto owned = std::move(*it);
				roots.erase(it);
				return owned;
			}
		}

		// search recursively in children
		std::function<std::unique_ptr<GameObject>(GameObject*)> searchChildren;
		searchChildren = [&](GameObject* node) -> std::unique_ptr<GameObject>
			{
				for (auto& child : node->children())
				{
					if (child.get() == target)
						return node->RemoveChild(target);

					auto result = searchChildren(child.get());
					if (result) return result;
				}
				return nullptr;
			};

		for (auto& root : roots)
		{
			auto result = searchChildren(root.get());
			if (result) return result;
		}

		return nullptr;
	}

	void DrawGameObjectNode(GameObject* go, Scene& scene, int depth = 0)
	{
		if (!go) return; // guard against null

		bool hasChildren = !go->children().empty();
		bool isSelected = std::find(Editor::selectedObjects.begin(),
			Editor::selectedObjects.end(), go) != Editor::selectedObjects.end();

		ImGuiTreeNodeFlags flags =
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_FramePadding;

		if (isSelected)   flags |= ImGuiTreeNodeFlags_Selected;
		if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;

		bool nodeOpen = ImGui::TreeNodeEx(go->name().c_str(), flags);

		// selection
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			//selecting multiple individually
			if (ImGui::GetIO().KeyCtrl)
			{
				auto it = std::find(Editor::selectedObjects.begin(), Editor::selectedObjects.end(), go);
				if (it != Editor::selectedObjects.end()) 
					Editor::selectedObjects.erase(it);
				else 
					Editor::selectedObjects.push_back(go);
			}
			//selecting range
			else if (ImGui::GetIO().KeyShift && !Editor::selectedObjects.empty())
			{
				auto start = std::find_if(scene.gameObjectList().begin(), scene.gameObjectList().end(),
					[&](const std::unique_ptr<GameObject>& obj)
					{
						return obj.get() == Editor::selectedObjects.back();
					});

				auto end = std::find_if(scene.gameObjectList().begin(), scene.gameObjectList().end(),
					[&](const std::unique_ptr<GameObject>& obj)
					{
						return obj.get() == go;
					});
				Editor::selectedObjects.clear();

				for (auto j = std::min(start, end); j <= std::max(start, end); j++)
				{
					Editor::selectedObjects.push_back(j->get());
				}
			}
			//selecting single
			else
			{
				bool alreadySelected = std::find(Editor::selectedObjects.begin(),
					Editor::selectedObjects.end(), go) != Editor::selectedObjects.end();
				if (!alreadySelected)
					Editor::selectedObjects = { go };
			}
		}

		// collapse to single on release without drag
		if (ImGui::IsItemDeactivated())
		{
			bool alreadySelected = std::find(Editor::selectedObjects.begin(),
				Editor::selectedObjects.end(), go) != Editor::selectedObjects.end();
			if (alreadySelected && !ImGui::IsMouseDragging(0))
				Editor::selectedObjects = { go };
		}

		// drag source
		if (ImGui::BeginDragDropSource())
		{
			if (!isSelected)
				Editor::selectedObjects = { go };

			ImGui::SetDragDropPayload("GO_DRAG", nullptr, 0);
			if (Editor::selectedObjects.size() > 1)
				ImGui::Text("Moving %d objects", (int)Editor::selectedObjects.size());
			else
				ImGui::TextUnformatted(go->name().c_str());
			ImGui::EndDragDropSource();
		}

		// drop indicator
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)
			&& ImGui::GetDragDropPayload() != nullptr)
		{
			bool isDraggedObj = std::find(Editor::selectedObjects.begin(),
				Editor::selectedObjects.end(), go) != Editor::selectedObjects.end();
			if (!isDraggedObj)
				DrawDropIndicator(GetDropZone());
		}

		// drop target
		if (ImGui::BeginDragDropTarget())
		{
			if (ImGui::AcceptDragDropPayload("GO_DRAG"))
			{
				DropZone zone = GetDropZone();

				// snapshot selection before modifying anything
				std::vector<GameObject*> toMove = Editor::selectedObjects;

				for (GameObject* dragged : toMove)
				{
					if (go->IsDescendantOf(dragged) || go == dragged) continue;

					auto owned = ExtractFromScene(scene, dragged);
					if (!owned) continue;

					if (zone == DropZone::Into)
					{
						go->AddChild(std::move(owned));
					}
					else
					{
						GameObject* targetParent = go->parent();
						auto& targetList = targetParent ?
							targetParent->children() : scene.gameObjectList();

						auto it = std::find_if(targetList.begin(), targetList.end(),
							[go](const std::unique_ptr<GameObject>& p) { return p.get() == go; });

						if (it != targetList.end())
						{
							owned->SetParent(targetParent);
							if (zone == DropZone::Below) ++it;
							targetList.insert(it, std::move(owned));
						}
						else
						{
							scene.gameObjectList().push_back(std::move(owned));
						}
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

		// recurse into children
		if (nodeOpen)
		{
			if (hasChildren)
			{
				std::vector<GameObject*> childPtrs;
				for (auto& child : go->children())
				{
					if (child) childPtrs.push_back(child.get()); // skip null children
				}

				for (GameObject* child : childPtrs)
					DrawGameObjectNode(child, scene, depth + 1);
			}
			ImGui::TreePop(); // always call if nodeOpen is true
		}
	}
}


namespace //wrappers for drawing ui elements
{
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

		for (auto& go : scene.gameObjectList())
		{
			if (!go) continue; // skip null entries
			DrawGameObjectNode(go.get(), scene);
		}

		if (ImGui::BeginPopupContextWindow("SceneRightClickMenu"))
		{
			if (ImGui::MenuItem("Create GameObject"))
			{
				int index = (int)scene.gameObjectList().size();
				std::string name = "GameObject_" + std::to_string(index);
				auto newGo = std::make_unique<GameObject>(name);
				Editor::selectedObjects = { newGo.get() };
				scene.gameObjectList().push_back(std::move(newGo));
			}

			if (!Editor::selectedObjects.empty())
			{
				if (ImGui::MenuItem("Delete GameObject"))
				{
					for (GameObject* go : Editor::selectedObjects)
						ExtractFromScene(scene, go); // unique_ptr falls out of scope = deleted
					Editor::selectedObjects.clear();
				}

				if (ImGui::MenuItem("Unparent"))
				{
					for (GameObject* go : Editor::selectedObjects)
					{
						if (!go->parent()) continue;
						auto owned = ExtractFromScene(scene, go);
						if (owned) scene.gameObjectList().push_back(std::move(owned));
					}
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
		if (Editor::selectedObjects.empty()) { ImGui::End(); return; }
		GameObject& selectedObj = *Editor::selectedObjects[0];

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
