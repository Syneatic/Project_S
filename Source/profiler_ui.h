/*
Author: Yan Chun
Co-Author: Nil
*/
#pragma once

#include "profiler.h"

// ─────────────────────────────────────────────
//  ProfilerUI
//  Call Render() once per frame inside an
//  ImGui frame (after NewFrame, before Render).
// ─────────────────────────────────────────────

class ProfilerUI
{
public:
    void Render()
    {
        Profiler& prof = Profiler::Get();

        ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Profiler", nullptr, ImGuiWindowFlags_MenuBar))
        {
            ImGui::End();
            return;
        }

        //top menu
        if (ImGui::BeginMenuBar())
        {
            bool paused = prof.IsPaused();
            if (ImGui::MenuItem(paused ? "Resume" : "Pause"))
            {
                bool nowPaused = !paused;
                prof.SetPaused(nowPaused);
                if (nowPaused)
                {
                    //snap to latest frame
                    const auto& fs = prof.Frames();
                    m_pausedAtFrame = m_selectedFrame >= 0 && m_selectedFrame < (int)fs.size()
                                        ? m_selectedFrame
                                        : (int)fs.size() - 1;
                    m_selectedFrame = m_pausedAtFrame;
                }
                else
                {
                    // Resume — stop pinning
                    m_pausedAtFrame = -1;
                    m_selectedFrame = -1;
                }
            }

            if (ImGui::MenuItem("Clear"))
            {
                m_selectedFrame = -1;
                m_pausedAtFrame = -1;
            }

            if (prof.IsPaused())
            {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "PAUSED");
            }

            ImGui::EndMenuBar();
        }

        const auto& frames = prof.Frames();
        if (frames.empty())
        {
            ImGui::TextDisabled("No frame data yet.");
            ImGui::End();
            return;
        }

        RenderFrameSelector(frames);

        ImGui::Separator();

        //choose which frame to display
        int displayIdx = m_selectedFrame;
        if (displayIdx < 0 || displayIdx >= (int)frames.size())
        {
            //follow latest frame if resume
            displayIdx = (int)frames.size() - 1;
        }
        const FrameData& fd = frames[displayIdx];

        ImGui::TextUnformatted(std::to_string(AEFrameRateControllerGetFrameRate()).c_str());

        if (ImGui::BeginTabBar("ProfilerTabs"))
        {
            if (ImGui::BeginTabItem("Tree View"))
            {
                RenderTreeView(fd);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }

private:
    int   m_selectedFrame  = -1;
    int   m_pausedAtFrame  = -1;     // frame index snapped at pause time
    float m_timelineZoom   = 1.0f;   // ms per pixel scale
    float m_timelineScroll = 0.0f;

    // ── Frame selector ───────────────────────────────────────────────

    void RenderFrameSelector(const std::vector<FrameData>& frames)
    {
        const float graphH    = 50.0f;
        const float barW      = 6.0f;
        const float targetMs  = 16.667f; // 60 fps line
        ImDrawList* dl        = ImGui::GetWindowDrawList();
        ImVec2      cursor    = ImGui::GetCursorScreenPos();
        float       availW    = ImGui::GetContentRegionAvail().x;

        if (availW <= 0.f) return;

        ImGui::InvisibleButton("##frame_sel", ImVec2(availW, graphH));
        bool hovered = ImGui::IsItemHovered();

        // Background
        dl->AddRectFilled(cursor,
                          ImVec2(cursor.x + availW, cursor.y + graphH),
                          IM_COL32(30, 30, 30, 255));

        // 60fps reference line
        float refY = cursor.y + graphH - (targetMs / 33.333f) * graphH;
        dl->AddLine(ImVec2(cursor.x, refY),
                    ImVec2(cursor.x + availW, refY),
                    IM_COL32(80, 200, 80, 120), 1.0f);

        int count = (int)frames.size();
        int startIdx = std::max(0, count - (int)(availW / barW));

        for (int i = startIdx; i < count; ++i)
        {
            float t   = (float)std::min(frames[i].frameTimeMs / 33.333, 1.0);
            float x0  = cursor.x + (i - startIdx) * barW;
            float y0  = cursor.y + graphH - t * graphH;
            float x1  = x0 + barW - 1.0f;
            float y1  = cursor.y + graphH;

            uint32_t col = frames[i].frameTimeMs > targetMs
                ? IM_COL32(220, 80, 60, 255)
                : IM_COL32(60, 180, 100, 255);

            if (i == m_selectedFrame)
                col = IM_COL32(255, 220, 50, 255);

            dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), col);

            // Click to select frame
            if (hovered && ImGui::IsMouseClicked(0))
            {
                float mx = ImGui::GetIO().MousePos.x;
                if (mx >= x0 && mx < x1)
                {
                    m_selectedFrame = i;
                }
            }
        }

        // Tooltip
        if (hovered)
        {
            float mx = ImGui::GetIO().MousePos.x;
            int idx  = startIdx + (int)((mx - cursor.x) / barW);
            if (idx >= 0 && idx < count)
            {
                ImGui::SetTooltip("Frame %d  %.2f ms  (%.0f fps)",
                    idx,
                    frames[idx].frameTimeMs,
                    1000.0 / std::max(frames[idx].frameTimeMs, 0.001));
            }
        }

        ImGui::Dummy(ImVec2(availW, 2)); // spacing after graph
    }

    // ── Tree View ────────────────────────────────────────────────────

    struct TreeNode
    {
        std::string          name;
        double               totalMs   = 0.0;
        double               selfMs    = 0.0;
        int                  callCount = 0;
        std::vector<TreeNode> children;
    };

    void RenderTreeView(const FrameData& fd)
    {
        if (fd.samples.empty())
        {
            ImGui::TextDisabled("No samples.");
            return;
        }

        std::vector<TreeNode> roots = BuildTree(fd.samples);

        ImGui::Columns(4, "tree_cols");
        ImGui::SetColumnWidth(0, 300);
        ImGui::SetColumnWidth(1, 120);
        ImGui::SetColumnWidth(2, 120);
        ImGui::SetColumnWidth(3, 80);

        ImGui::TextUnformatted("Name");    ImGui::NextColumn();
        ImGui::TextUnformatted("Total ms");ImGui::NextColumn();
        ImGui::TextUnformatted("Self ms"); ImGui::NextColumn();
        ImGui::TextUnformatted("Calls");   ImGui::NextColumn();
        ImGui::Separator();

        for (auto& root : roots)
            RenderTreeNode(root, fd.frameTimeMs);

        ImGui::Columns(1);
    }

    void RenderTreeNode(const TreeNode& node, double frameMs, int depth = 0)
    {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
        if (node.children.empty())
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

        // Color bar proportional to total time
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float  pct = (float)(node.totalMs / std::max(frameMs, 0.001));
        float  barW = pct * 280.0f;
        ImGui::GetWindowDrawList()->AddRectFilled(
            pos, ImVec2(pos.x + barW, pos.y + ImGui::GetTextLineHeight()),
            IM_COL32(80, 140, 200, 60));

        bool open = ImGui::TreeNodeEx(node.name.c_str(), flags);

        ImGui::NextColumn();
        // Total ms + per-call average when called multiple times
        if (node.callCount > 1)
        {
            ImGui::Text("%.3f ms", node.totalMs);
            ImGui::SameLine();
            ImGui::TextDisabled("(%.3f avg)", node.totalMs / node.callCount);
        }
        else
        {
            ImGui::Text("%.3f ms", node.totalMs);
        }
        ImGui::NextColumn();
        ImGui::Text("%.3f ms", node.selfMs);   ImGui::NextColumn();
        // Color the call count orange if > 1 to draw attention
        if (node.callCount > 1)
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.2f, 1.0f), "%d", node.callCount);
        else
            ImGui::Text("%d", node.callCount);
        ImGui::NextColumn();

        if (open && !node.children.empty())
        {
            for (auto& child : node.children)
                RenderTreeNode(child, frameMs, depth + 1);
            ImGui::TreePop();
        }
    }

    // Build a call tree from flat sample list.
    // Samples are ordered by start time; depth encodes nesting.
    std::vector<TreeNode> BuildTree(const std::vector<ProfileSample>& samples)
    {
        // Sort by start time so parents always precede their children
        std::vector<const ProfileSample*> sorted;
        sorted.reserve(samples.size());
        for (auto& s : samples) sorted.push_back(&s);
        std::stable_sort(sorted.begin(), sorted.end(),
            [](const ProfileSample* a, const ProfileSample* b)
            { return a->startMs < b->startMs; });

        // Build an unmerged tree using a depth stack
        std::vector<TreeNode*> stack;
        std::vector<TreeNode>  roots;

        for (auto* sp : sorted)
        {
            TreeNode node;
            node.name      = sp->name;
            node.totalMs   = sp->durationMs;
            node.callCount = 1;

            // Pop back to the correct parent depth
            while (!stack.empty() && (int)stack.size() > sp->depth)
                stack.pop_back();

            if (stack.empty())
            {
                roots.push_back(std::move(node));
                stack.push_back(&roots.back());
            }
            else
            {
                TreeNode* parent = stack.back();
                parent->children.push_back(std::move(node));
                stack.push_back(&parent->children.back());
            }
        }

        // Merge repeated same-named siblings (fixed-timestep physics calls etc.)
        MergeSiblings(roots);
        ComputeSelfTime(roots);
        return roots;
    }

    // Merges sibling nodes that share the same name into one aggregated node.
    // totalMs and callCount are accumulated; children lists are concatenated
    // then recursively merged, so sub-scopes inside each Physics tick are
    // also correctly aggregated (e.g. Broadphase x3, NarrowPhase x3).
    void MergeSiblings(std::vector<TreeNode>& nodes)
    {
        std::vector<TreeNode> merged;
        merged.reserve(nodes.size());

        for (auto& node : nodes)
        {
            TreeNode* existing = nullptr;
            for (auto& m : merged)
                if (m.name == node.name) { existing = &m; break; }

            if (existing)
            {
                existing->totalMs   += node.totalMs;
                existing->callCount += node.callCount;
                for (auto& child : node.children)
                    existing->children.push_back(std::move(child));
            }
            else
            {
                merged.push_back(std::move(node));
            }
        }

        nodes = std::move(merged);

        // Recurse so children are merged too
        for (auto& n : nodes)
            MergeSiblings(n.children);
    }

    void ComputeSelfTime(std::vector<TreeNode>& nodes)
    {
        for (auto& n : nodes)
        {
            double childrenTotal = 0.0;
            for (auto& c : n.children) childrenTotal += c.totalMs;
            n.selfMs = std::max(0.0, n.totalMs - childrenTotal);
            ComputeSelfTime(n.children);
        }
    }

    // ── Helpers ──────────────────────────────────────────────────────

    static uint32_t BrightenColor(uint32_t col)
    {
        uint8_t r = (uint8_t)std::min(255, (int)((col >>  0) & 0xFF) + 50);
        uint8_t g = (uint8_t)std::min(255, (int)((col >>  8) & 0xFF) + 50);
        uint8_t b = (uint8_t)std::min(255, (int)((col >> 16) & 0xFF) + 50);
        return (col & 0xFF000000) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | r;
    }
};
