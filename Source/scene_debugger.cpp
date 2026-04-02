#ifdef _DEBUG

#include "renderer.hpp"
#include "camera.hpp"
#include "gameobject.hpp"
#include "scene_debugger.hpp"
#include "scene_manager.hpp"
#include "components.hpp"

namespace
{
    GameObject* _hovered{ nullptr };
    GameObject* _locked{ nullptr };
    bool        _active{ false };
    int         _warmupFrames{ 0 };
    int _inspectorSettledFrames{ 0 };

    // ── Error popup state ────────────────────────────────────────
    std::string _errorMsg{};
    bool        _showError{ false };

    // ── Hit testing ─────────────────────────────────────────────

    bool HitTest(const GameObject& go, float2 worldMouse)
    {
        const Transform& wt = go.worldTransform();
        float halfW = std::max(std::abs(wt.scale.x) * 0.5f, 0.15f);
        float halfH = std::max(std::abs(wt.scale.y) * 0.5f, 0.15f);
        return worldMouse.x >= wt.position.x - halfW &&
            worldMouse.x <= wt.position.x + halfW &&
            worldMouse.y >= wt.position.y - halfH &&
            worldMouse.y <= wt.position.y + halfH;
    }

    GameObject* PickRecursive(GameObject& go, float2 worldMouse)
    {
        for (auto& child : go.children())
            if (auto* hit = PickRecursive(*child, worldMouse))
                return hit;
        return HitTest(go, worldMouse) ? &go : nullptr;
    }

    GameObject* PickFromScene(Scene& scene, float2 worldMouse)
    {
        for (auto& pgo : scene.gameObjectList())
            if (auto* hit = PickRecursive(*pgo, worldMouse))
                return hit;
        return nullptr;
    }

    // ── Outline drawing ─────────────────────────────────────────
    // Called BEFORE Graphics::Execute() so outlines appear same frame.

    void DrawOutline(const GameObject& go, Color col)
    {
        Graphics::RenderData rd{};
        rd.alignment = Graphics::Alignment::MC;
        rd.blendMode = Graphics::BlendMode::AE_GFX_BM_NONE;
        rd.drawMode = Graphics::DrawMode::AE_GFX_MDM_LINES;
        rd.color = col;
        rd.layer = (Graphics::RenderLayer)(Graphics::RenderLayer::GIZMOS + 20);
        rd.pos = go.worldTransform().position;
        rd.scale = go.worldTransform().scale;
        rd.rot = go.worldTransform().rotation;
        Graphics::Submit(rd, Graphics::PrimitiveType::BOX);
    }

    // ── ImGui helpers ────────────────────────────────────────────

    void ROFloat2(const char* label, const float2& v)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(label);
        ImGui::TableSetColumnIndex(1); ImGui::Text("X: %.3f   Y: %.3f", v.x, v.y);
    }

    void ROFloat(const char* label, float v)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(label);
        ImGui::TableSetColumnIndex(1); ImGui::Text("%.3f", v);
    }

    bool BeginPropTable(const char* id)
    {
        if (ImGui::GetContentRegionAvail().x <= 0.f) return false;
        return ImGui::BeginTable(id, 2,
            ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchSame);
    }


    // ── UI panels ────────────────────────────────────────────────
    // These are called INSIDE an already-open ImGui frame.

    void BuildMenuBar(Scene& scene)
    {
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        if (vp->WorkSize.x <= 0.f || vp->WorkSize.y <= 0.f) return;

        ImGui::BeginMainMenuBar();
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.15f, 0.05f, 0.25f, 1.f));

        if (ImGui::BeginMenu("Debugger"))
        {
            ImGui::TextDisabled("Read-only overlay  [F5 to hide]");
            ImGui::Separator();
            if (ImGui::MenuItem("Switch to Editor"))
                SceneManager::SwitchToEditor();
            ImGui::Separator();
            if (ImGui::MenuItem("Quit"))
                EngineCTX::applicationRunning = false;
            ImGui::EndMenu();
        }

        // Centred scene label
        {
            std::string label = "[DEBUG]  scene: " + scene.name();
            float textW = ImGui::CalcTextSize(label.c_str()).x;
            float barW = ImGui::GetContentRegionAvail().x;
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (barW - textW) * 0.5f);
            ImGui::TextDisabled("%s", label.c_str());
        }

        ImGui::PopStyleColor();
        ImGui::EndMainMenuBar();
    }

    void DrawNode(GameObject* go)
    {
        if (go->cname().empty()) return;

        bool hasChildren = !go->children().empty();
        bool isTarget = (go == _locked || (!_locked && go == _hovered));

        ImGuiTreeNodeFlags flags =
            ImGuiTreeNodeFlags_OpenOnArrow |
            ImGuiTreeNodeFlags_SpanAvailWidth |
            ImGuiTreeNodeFlags_FramePadding |
            ImGuiTreeNodeFlags_DrawLinesToNodes;
        flags |= hasChildren ? 0 : ImGuiTreeNodeFlags_Leaf;
        if (isTarget) flags |= ImGuiTreeNodeFlags_Selected;

        if (isTarget)
        {
            ImGui::PushStyleColor(ImGuiCol_Header,
                ImVec4(0.f, 0.55f, 0.55f, 0.55f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                ImVec4(0.f, 0.75f, 0.75f, 0.65f));
        }

        bool nodeOpen = ImGui::TreeNodeEx(go->cname().c_str(), flags);

        if (isTarget) ImGui::PopStyleColor(2);

        // Click in hierarchy = lock/unlock
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            _locked = (_locked == go) ? nullptr : go;

        if (ImGui::IsItemHovered())
        {
            const Transform& wt = go->worldTransform();
            ImGui::SetTooltip(
                "World pos   (%.2f, %.2f)\n"
                "World scale (%.2f, %.2f)\n"
                "World rot    %.2f deg",
                wt.position.x, wt.position.y,
                wt.scale.x, wt.scale.y,
                wt.rotation);
        }

        if (nodeOpen)
        {
            for (auto& child : go->children())
                DrawNode(child.get());
            ImGui::TreePop();
        }
    }

    void BuildHierarchyWindow(Scene& scene)
    {
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        if (vp->WorkSize.x <= 0.f || vp->WorkSize.y <= 0.f) return;
        ImGui::SetNextWindowSizeConstraints(ImVec2(280.f, 100.f), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Hierarchy##dbg");
        ImGui::TextUnformatted(scene.name().c_str());
        ImGui::Separator();
        for (auto& pgo : scene.gameObjectList())
            DrawNode(pgo.get());
        ImGui::End();
    }

    void BuildInspectorWindow()
    {
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        if (vp->WorkSize.x <= 0.f || vp->WorkSize.y <= 0.f) return;

        GameObject* target = _locked ? _locked : _hovered;

        ImGui::SetNextWindowSizeConstraints(ImVec2(300.f, 100.f), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Inspector##dbg");

        if (!target)
        {
            ImGui::TextDisabled("Hover or click a GameObject to inspect.");
            ImGui::End();
            return;
        }

        // Title row
        if (_locked)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.4f, 1.f, 1.f));
            ImGui::TextUnformatted(u8"\u25CF LOCKED");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            if (ImGui::SmallButton("Unlock")) _locked = nullptr;
            ImGui::SameLine();
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 1.f, 1.f, 1.f));
            ImGui::TextUnformatted(u8"\u25CB HOVERING");
            ImGui::PopStyleColor();
            ImGui::SameLine();
        }

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 1.f, 1.f, 1.f));
        ImGui::TextUnformatted(target->cname().c_str());
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("id: %zu", target->id);

        {
            bool active = target->active();
            ImGui::BeginDisabled(true);
            ImGui::Checkbox("Active##dbg_go", &active);
            ImGui::EndDisabled();
        }

        ImGui::Separator();

        // Local transform
        if (ImGui::CollapsingHeader("Transform (local)", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const Transform& lt = target->transform();
            if (BeginPropTable("##lt"))
            {
                ROFloat2("Position", lt.position);
                ROFloat2("Scale", lt.scale);
                ROFloat("Rotation", lt.rotation);
                ImGui::EndTable();
            }
        }

        // World transform
        if (ImGui::CollapsingHeader("Transform (world)", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const Transform& wt = target->worldTransform();
            if (BeginPropTable("##wt"))
            {
                ROFloat2("Position", wt.position);
                ROFloat2("Scale", wt.scale);
                ROFloat("Rotation", wt.rotation);
                ImGui::EndTable();
            }
        }

        ImGui::Separator();

        // Components
        const auto& comps = target->componentMap();
        const auto& compOrder = target->componentOrder();

        if (compOrder.empty())
        {
            ImGui::TextDisabled("No components.");
            ImGui::End();
            return;
        }

        for (const std::type_index& type : compOrder)
        {
            auto it = comps.find(type);
            if (it == comps.end() || !it->second) continue;

            const Component* comp = it->second.get();
            bool open = ImGui::CollapsingHeader(
                comp->name().c_str(), ImGuiTreeNodeFlags_DefaultOpen);

            ImGui::SameLine();
            {
                bool active = comp->active();
                ImGui::BeginDisabled(true);
                std::string cbId = "##dbg_ca_" + comp->name();
                ImGui::Checkbox(cbId.c_str(), &active);
                ImGui::EndDisabled();
            }

            if (open)
            {
                float avail = ImGui::GetContentRegionAvail().x;
                if (avail > 1.f && _inspectorSettledFrames >= 2)  // must be truly settled
                {
                    ImGui::BeginDisabled(true);
                    const_cast<Component*>(comp)->DrawInInspector();
                    ImGui::EndDisabled();
                }
                ImGui::Separator();
            }
        }

        ImGui::End();
    }

    void BuildStatusBar()
    {
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        if (vp->WorkSize.x <= 0.f || vp->WorkSize.y <= 0.f) return;

        float statusH = 24.f;
        float winW = std::max(vp->WorkSize.x, 1.f); // ← clamp width
        float winY = vp->WorkPos.y + vp->WorkSize.y - statusH;

        ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x, winY));
        ImGui::SetNextWindowSize(ImVec2(winW, statusH));
        ImGui::SetNextWindowBgAlpha(0.75f);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings;

        ImGui::Begin("##dbg_status", nullptr, flags);

        if (_locked)
        {
            const Transform& wt = _locked->worldTransform();
            ImGui::Text(u8"\u25CF LOCKED  %s  |  pos (%.2f, %.2f)  |  %zu component(s)  |  click to unlock",
                _locked->cname().c_str(), wt.position.x, wt.position.y,
                _locked->componentOrder().size());
        }
        else if (_hovered)
        {
            const Transform& wt = _hovered->worldTransform();
            ImGui::Text(u8"\u25CB HOVER  %s  |  pos (%.2f, %.2f)  |  click to lock",
                _hovered->cname().c_str(), wt.position.x, wt.position.y);
        }
        else
        {
            ImGui::TextDisabled("DEBUG MODE  |  hover a GameObject to inspect  |  F5 to toggle");
        }

        ImGui::End();
    }

    void BuildErrorPopup()
    {
        if (!_showError) return;

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        if (vp->WorkSize.x <= 0.f || vp->WorkSize.y <= 0.f) return; // ADD THIS

        ImGui::SetNextWindowPos(vp->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500.f, 200.f));

        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.7f, 0.1f, 0.1f, 1.f));
        ImGui::Begin("Runtime Error##dbgerr", nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoDocking);
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.35f, 0.35f, 1.f));
        ImGui::TextUnformatted("An error occurred at runtime:");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::TextWrapped("%s", _errorMsg.c_str());
        ImGui::Spacing();
        ImGui::Separator();

        // Guard against zero content width
        float btnW = std::max(ImGui::GetContentRegionAvail().x, 1.f); // CHANGE THIS
        if (ImGui::Button("Dismiss##dbgerr", ImVec2(btnW, 0.f)))
            _showError = false;

        ImGui::End();
    }

}
namespace Debugger
{
    bool IsActive() { return _active; }

    void ReportError(const std::string& msg)
    {
        _errorMsg = msg;
        _showError = true;
        Debug::Log("Debugger error: ", msg);
    }

    void Reset()
    {
        _hovered = nullptr;
        _locked = nullptr;
    }

    void Tick(Scene& scene)
    {
        bool wasActive = _active;
        _active = EngineCTX::debugMode;

        if (!_active)
        {
            _hovered = nullptr;
            _warmupFrames = 3;   // reset so next toggle-on gets 3 skip frames
            return;
        }

        // On fresh toggle-on, start warmup countdown
        if (!wasActive)
            _warmupFrames = 3;

        // During warmup: skip ALL ImGui panels, just tick down
        if (_warmupFrames > 0)
        {
            --_warmupFrames;
            _inspectorSettledFrames = 0;  // reset settled counter during warmup
            return;
        }

        if (!ImGui::GetIO().WantCaptureMouse)
        {
            s32 mx, my;
            AEInputGetCursorPosition(&mx, &my);
            float2 worldMouse = CameraSystem::ScreenToWorld(
                { static_cast<f32>(mx), static_cast<f32>(my) });

            _hovered = PickFromScene(scene, worldMouse);
            if (AEInputCheckTriggered(AEVK_LBUTTON))
                _locked = _hovered
                ? ((_locked == _hovered) ? nullptr : _hovered)
                : nullptr;
        }

        // ── 2. Outlines ───────────────────────────────────────────
        if (_locked)
            DrawOutline(*_locked, Color(0xFF'CC'55'FF));
        else if (_hovered)
            DrawOutline(*_hovered, Color(0xFF'00'FF'FF));

        // ── 3. ImGui panels ───────────────────────────────────────
        if (ImGui::GetMainViewport()->WorkSize.x <= 0.f) return;
        BuildMenuBar(scene);
        BuildHierarchyWindow(scene);
        BuildInspectorWindow();
        BuildStatusBar();
        BuildErrorPopup();
        _inspectorSettledFrames++;
    }
 }

#endif // _DEBUG