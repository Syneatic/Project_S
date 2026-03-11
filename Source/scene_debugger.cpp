// scene_debugger.cpp
// Read-only overlay debugger.
// Compiled only in _DEBUG builds.
#ifdef _DEBUG

// systems
#include "renderer.hpp"
#include "camera.hpp"
#include "gameobject.hpp"

// scene
#include "scene_debugger.hpp"
#include "scene_manager.hpp"

// comps
#include "components.hpp"

// ================================================================
//  State
// ================================================================
namespace
{
    GameObject* _hovered{ nullptr };  // object under mouse this frame
    GameObject* _locked{ nullptr };   // clicked and pinned

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
        return ImGui::BeginTable(id, 2,
            ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchSame);
    }

    // ── UI panels ────────────────────────────────────────────────

    void BuildDockSpace()
    {
        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
            ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground;

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
        ImGui::Begin("##dbg_dock", nullptr, flags);
        ImGui::PopStyleVar(2);
        ImGui::DockSpace(ImGui::GetID("DbgDockSpace"), ImVec2(0, 0),
            ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::End();
    }

    void BuildMenuBar(Scene& scene)
    {
        ImGui::BeginMainMenuBar();
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.15f, 0.05f, 0.25f, 1.f));

        if (ImGui::BeginMenu("Debugger"))
        {
            ImGui::TextDisabled("Read-only overlay");
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

    // Recursively draw read-only hierarchy node
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

        // Teal tint for the inspected object
        if (isTarget)
        {
            ImGui::PushStyleColor(ImGuiCol_Header,
                ImVec4(0.f, 0.55f, 0.55f, 0.55f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered,
                ImVec4(0.f, 0.75f, 0.75f, 0.65f));
        }

        bool nodeOpen = ImGui::TreeNodeEx(go->cname().c_str(), flags);

        if (isTarget) ImGui::PopStyleColor(2);

        // Click in hierarchy also locks
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            if (_locked == go)
                _locked = nullptr;   // click same → unlock
            else
                _locked = go;
        }

        // Quick world-pos tooltip
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
        // Show locked object if pinned, otherwise hovered preview
        GameObject* target = _locked ? _locked : _hovered;

        ImGui::SetNextWindowSizeConstraints(ImVec2(300.f, 100.f), ImVec2(FLT_MAX, FLT_MAX));
        ImGui::Begin("Inspector##dbg");

        if (!target)
        {
            ImGui::TextDisabled("Hover or click a GameObject to inspect.");
            ImGui::End();
            return;
        }

        // ── Title bar ────────────────────────────────────────────
        if (_locked)
        {
            // Purple pin indicator when locked
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.4f, 1.f, 1.f));
            ImGui::TextUnformatted(u8"\u25CF LOCKED");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            if (ImGui::SmallButton("Unlock"))
                _locked = nullptr;
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

        // ── Local transform ──────────────────────────────────────
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

        // ── World transform ──────────────────────────────────────
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

        // ── Components ───────────────────────────────────────────
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

            // Read-only active badge
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
                // BeginDisabled makes every widget non-interactive
                ImGui::BeginDisabled(true);
                const_cast<Component*>(comp)->DrawInInspector();
                ImGui::EndDisabled();
                ImGui::Separator();
            }
        }

        ImGui::End();
    }

    void BuildStatusBar()
    {
        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(
            ImVec2(vp->WorkPos.x, vp->WorkPos.y + vp->WorkSize.y - 24.f));
        ImGui::SetNextWindowSize(ImVec2(vp->WorkSize.x, 24.f));
        ImGui::SetNextWindowBgAlpha(0.75f);

        ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
            ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings;

        ImGui::Begin("##dbg_status", nullptr, flags);

        if (_locked)
        {
            const Transform& wt = _locked->worldTransform();
            ImGui::Text(
                u8"\u25CF LOCKED  %s  |  pos (%.2f, %.2f)  |  %zu component(s)  "
                "| click object or [Unlock] to release",
                _locked->cname().c_str(),
                wt.position.x, wt.position.y,
                _locked->componentOrder().size());
        }
        else if (_hovered)
        {
            const Transform& wt = _hovered->worldTransform();
            ImGui::Text(
                u8"\u25CB HOVER  %s  |  pos (%.2f, %.2f)  |  click to lock",
                _hovered->cname().c_str(),
                wt.position.x, wt.position.y);
        }
        else
        {
            ImGui::TextDisabled("DEBUG MODE  |  hover a GameObject to inspect");
        }

        ImGui::End();
    }

} // anonymous namespace


// ================================================================
//  Public entry point
// ================================================================
namespace Debugger
{
    void Tick(Scene& scene)
    {
        // ── 1. Pick hovered object (skip if ImGui owns mouse) ────
        if (!ImGui::GetIO().WantCaptureMouse)
        {
            s32 mx, my;
            AEInputGetCursorPosition(&mx, &my);
            float2 worldMouse = CameraSystem::ScreenToWorld(
                { static_cast<f32>(mx), static_cast<f32>(my) });

            _hovered = PickFromScene(scene, worldMouse);

            // Left click in viewport
            if (AEInputCheckTriggered(AEVK_LBUTTON))
            {
                if (_hovered)
                    _locked = (_locked == _hovered) ? nullptr : _hovered;
                else
                    _locked = nullptr;  // clicked empty space → unlock
            }
        }

        // ── 2. Draw outlines (submitted before Graphics::Execute
        //       already ran, so we re-submit here for next frame;
        //       or call Graphics::Execute again if your pipeline
        //       allows it — adjust to match your render order) ────
        if (_locked)
            DrawOutline(*_locked, Color(0xFF'CC'55'FF));   // purple = locked
        else if (_hovered)
            DrawOutline(*_hovered, Color(0xFF'00'FF'FF));  // cyan   = hover

        // ── 3. ImGui overlay ─────────────────────────────────────
        if (!EngineCTX::imguiInitialize) return;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        BuildDockSpace();
        BuildMenuBar(scene);
        BuildHierarchyWindow(scene);
        BuildInspectorWindow();
        BuildStatusBar();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        ImGui::EndFrame();
    }
}

#endif // _DEBUG