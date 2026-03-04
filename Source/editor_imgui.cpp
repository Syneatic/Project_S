//systems
#include "physics.hpp"
#include "gameobject.hpp"
#include "audio.hpp"
#include "camera.hpp"

//scene
#include "scene_parser.hpp"
#include "scene_editor.hpp"
#include "imgui_helper.hpp"
#include "editor.hpp"

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

	std::vector<GameObject*> _visibleList{};

	std::unique_ptr<GameObject> ExtractFromRoot(Scene& scene, GameObject* go)
	{
		auto& list = scene.gameObjectList();
		auto it = std::find_if(list.begin(), list.end(),
			[go](const std::unique_ptr<GameObject>& p) { return p.get() == go; });
		if (it == list.end()) return nullptr;
		auto owned = std::move(*it);
		list.erase(it);
		return owned;
	}

	std::unique_ptr<GameObject> DetachGO(Scene& scene, GameObject* go)
	{
		if (go->parent())
			return go->parent()->RemoveChild(go);
		return ExtractFromRoot(scene, go);
	}

	bool IsSelected(GameObject* go)
	{
		auto it = std::find(Editor::selectedObjects.begin(), Editor::selectedObjects.end(), go);
		return it != Editor::selectedObjects.end();
	}

	void SelectInteraction(GameObject* go,bool isSelected)
	{
		if (ImGui::GetDragDropPayload()) return;

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
		{
			if (ImGui::GetIO().KeyShift && !Editor::selectedObjects.empty()) //make sure not empty
			{
				GameObject* pivotptr = Editor::selectedObjects[0];
				auto pivot = std::find(_visibleList.begin(), _visibleList.end(), pivotptr);
				auto target = std::find(_visibleList.begin(), _visibleList.end(), go);

				auto [first, last] = std::minmax(pivot, target);
				for (auto it = first; it <= last; ++it)
				{
					if(IsSelected(*it)) continue;
					Editor::selectedObjects.push_back(*it);
				}
				
			}
			else if (ImGui::GetIO().KeyCtrl)
			{
				if (isSelected)
				{
					auto it = std::remove(Editor::selectedObjects.begin(), Editor::selectedObjects.end(), go);
					//remove it
					Editor::selectedObjects.erase(it);
				}
				else
				{
					//add
					Editor::selectedObjects.push_back(go);
				}
			}
			else
			{
				if (!isSelected || Editor::selectedObjects.size() == 1)
				{
					Editor::selectedObjects.clear();
					Editor::selectedObjects.push_back(go);
				}
			}
		}
	}

	enum class DropZone { Above, Into, Below };

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
			fg->AddLine(ImVec2(minX, minY), ImVec2(maxX, minY), IM_COL32(255, 120, 0, 255), 2.f);
			fg->AddCircleFilled(ImVec2(minX, minY), 3.f, IM_COL32(255, 120, 0, 255));
		}
		else if (zone == DropZone::Below)
		{
			fg->AddLine(ImVec2(minX, maxY), ImVec2(maxX, maxY), IM_COL32(255, 120, 0, 255), 2.f);
			fg->AddCircleFilled(ImVec2(minX, maxY), 3.f, IM_COL32(255, 120, 0, 255));
		}
		else if (zone == DropZone::Into)
		{
			fg->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY), IM_COL32(255, 120, 0, 255), 0.f, 0, 2.f);
		}
	}

	void DrawGameObjectNode(Scene& scene,GameObject* go)
	{
		bool hasChildren = !go->children().empty();
		bool isSelected = IsSelected(go);

		//set how to draw the node
		ImGuiTreeNodeFlags flags =
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			ImGuiTreeNodeFlags_FramePadding;

		flags |= ImGuiTreeNodeFlags_DrawLinesToNodes;
		flags |= isSelected ? ImGuiTreeNodeFlags_Selected : 0;
		flags |= hasChildren ? 0 : ImGuiTreeNodeFlags_Leaf;

		bool nodeOpen = ImGui::TreeNodeEx(go->cname().c_str(), flags);

		//check drop
		if (ImGui::BeginDragDropTarget())
		{
			if (ImGui::AcceptDragDropPayload("GO_DRAG"))
			{
				DropZone zone = GetDropZone();
				//depending on where dropped is where to insert
				auto& list = go->parent() ? go->parent()->children() : scene.gameObjectList();

				for (auto so : Editor::selectedObjects)
				{
					/*if child -> extract from parent
					  if root -> just move around */
					std::unique_ptr<GameObject> ptr = DetachGO(scene, so);

					auto it = std::find_if(list.begin(), list.end(), [go](const std::unique_ptr<GameObject>& o)
						{
							return o.get() == go;
						});

					switch (zone)
					{
					case DropZone::Above:
						//find position in gameobject list
						list.insert(it,std::move(ptr));
						break;
					case DropZone::Below:
						list.insert(it + 1, std::move(ptr));
						break;
					case DropZone::Into:
						go->AddChild(std::move(ptr));
						break;
					default:
						continue;
					}
				}
				//Editor::selectedObjects.clear();
			}
			ImGui::EndDragDropTarget();
		}


		//do drag
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
		{
			ImGui::SetDragDropPayload("GO_DRAG", nullptr,0);

			if (IsSelected(go) && Editor::selectedObjects.size() > 1)
				ImGui::Text("Moving %d objects", (int)Editor::selectedObjects.size());
			else
				ImGui::TextUnformatted(go->cname().c_str());


			ImGui::EndDragDropSource();
		}

		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)
			&& ImGui::GetDragDropPayload() != nullptr)
		{
			bool isDraggedObj = std::find(Editor::selectedObjects.begin(),
				Editor::selectedObjects.end(), go) != Editor::selectedObjects.end();
			if (!isDraggedObj)
				DrawDropIndicator(GetDropZone());
		}

		//click interaction
		SelectInteraction(go, isSelected);

		//recursively draw child
		if (nodeOpen)
		{
			for (auto& childptr : go->children())
			{
				DrawGameObjectNode(scene,childptr.get());
			}
			ImGui::TreePop();
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
		ImGui::Begin("Scene##window");

		NameInputText(scene.name());
		ImGui::Separator();
		_visibleList.clear();
		
		for(auto& go : scene.gameObjectList())
			_visibleList.push_back(go.get());

		for (auto go : _visibleList)
		{
			DrawGameObjectNode(scene,go);
		}


		//scene ctx menu
		if (ImGui::BeginPopupContextWindow("SceneCTXMenu", ImGuiPopupFlags_MouseButtonRight))
		{
			if (ImGui::MenuItem("Create GameObject"))
			{
				std::unique_ptr<GameObject> newobj = std::make_unique<GameObject>("GameObject" + scene.gameObjectList().size());
				scene.gameObjectList().push_back(std::move(newobj));
			}


			if (!Editor::selectedObjects.empty())
			{
				if (ImGui::MenuItem("Delete Selected"))
				{
					for (auto go : Editor::selectedObjects)
					{
						DetachGO(scene, go);
					}
					Editor::selectedObjects.clear();
				}
			}
	
			ImGui::EndPopup();
		}
		
		ImGui::End();
	}

	void BuildInspectorWindow(Scene& scene)
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(320.f, 100.f), ImVec2(FLT_MAX, FLT_MAX));
		ImGui::Begin("Inspector##window");

		//check if an object is selected
		if (Editor::selectedObjects.empty()) { ImGui::End(); return; }
		GameObject& selectedObj = *Editor::selectedObjects[0];

		//display selected object's properties
		//iterate through each component and display its properties here
		//name text box
		auto& transform = selectedObj.transform();
		auto& wtransform = selectedObj.worldTransform();

		NameInputText(selectedObj.name());

		//draw transform
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Float2DragReset("Position", &transform.position.x, {0.f,0.f}, 0.05f);
			Float2DragReset("Scale", &transform.scale.x, { 1.f,1.f }, 0.05f);
			FloatDragReset("Rotation", &transform.rotation, 0.f, 0.1f);
			ImGui::Separator();
		}

		if (ImGui::CollapsingHeader("World Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Float2DragReset("Position", &wtransform.position.x, { 0.f,0.f }, 0.05f);
			Float2DragReset("Scale", &wtransform.scale.x, { 1.f,1.f }, 0.05f);
			FloatDragReset("Rotation", &wtransform.rotation, 0.f, 0.1f);
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
