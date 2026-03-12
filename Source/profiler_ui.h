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

        // ── Menu bar ────────────────────────────────────────────────
        if (ImGui::BeginMenuBar())
        {
            bool paused = prof.IsPaused();
            if (ImGui::MenuItem(paused ? "Resume" : "Pause"))
            {
                bool nowPaused = !paused;
                prof.SetPaused(nowPaused);
                if (nowPaused)
                {
                    // Snap to whatever frame is currently shown
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

        // ── Frame selector (mini frame-time graph) ───────────────────
        RenderFrameSelector(frames);

        ImGui::Separator();

        // Resolve which frame to display.
        // While paused: pin to m_pausedAtFrame unless the user explicitly
        // clicks a different bar in the frame selector.
        int displayIdx = m_selectedFrame;
        if (displayIdx < 0 || displayIdx >= (int)frames.size())
        {
            // Not paused and nothing selected — follow the latest frame
            displayIdx = (int)frames.size() - 1;
        }
        const FrameData& fd = frames[displayIdx];

        // ── Tabs ─────────────────────────────────────────────────────
        if (ImGui::BeginTabBar("ProfilerTabs"))
        {
            if (ImGui::BeginTabItem("Timeline"))
            {
                RenderTimeline(fd);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Tree View"))
            {
                RenderTreeView(fd);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Memory"))
            {
                RenderMemory(frames);
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
            float t   = std::min(frames[i].frameTimeMs / 33.333, 1.0);
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
                    m_selectedFrame = i;
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

    // ── Timeline ─────────────────────────────────────────────────────

    void RenderTimeline(const FrameData& fd)
    {
        if (fd.samples.empty())
        {
            ImGui::TextDisabled("No samples in this frame.");
            return;
        }

        // Controls
        ImGui::SliderFloat("Zoom", &m_timelineZoom, 0.1f, 20.0f, "%.2fx");
        ImGui::SameLine();
        ImGui::TextDisabled("(scroll to zoom, drag to pan)");

        const float rowH      = 22.0f;
        const float labelW    = 140.0f;
        const float rulerH    = 20.0f;

        // Find max depth and frame duration
        int    maxDepth    = 0;
        double frameDurMs  = fd.frameTimeMs > 0 ? fd.frameTimeMs : 16.667;
        for (auto& s : fd.samples)
            maxDepth = std::max(maxDepth, s.depth);

        float  totalH   = rulerH + (maxDepth + 1) * rowH + 4.0f;
        float  availW   = ImGui::GetContentRegionAvail().x - labelW;
        float  pxPerMs  = (availW / (float)frameDurMs) * m_timelineZoom;

        // Scroll region
        ImGui::BeginChild("##timeline_scroll", ImVec2(0, totalH + 20), false,
                          ImGuiWindowFlags_HorizontalScrollbar);

        ImDrawList* dl     = ImGui::GetWindowDrawList();
        ImVec2      origin = ImGui::GetCursorScreenPos();
        origin.x += labelW;

        float totalW = std::max(availW, pxPerMs * (float)frameDurMs);
        ImGui::InvisibleButton("##tl_area", ImVec2(labelW + totalW, totalH));

        // Zoom on scroll
        if (ImGui::IsItemHovered())
        {
            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.0f)
                m_timelineZoom = std::clamp(m_timelineZoom * (1.0f + wheel * 0.1f), 0.1f, 50.0f);
        }

        // Ruler
        {
            float rulerY = origin.y;
            dl->AddRectFilled(ImVec2(origin.x, rulerY),
                              ImVec2(origin.x + totalW, rulerY + rulerH),
                              IM_COL32(45, 45, 45, 255));

            float stepMs = 1.0f;
            if (pxPerMs < 10) stepMs = 5.0f;
            if (pxPerMs < 2)  stepMs = 10.0f;

            for (float ms = 0; ms <= (float)frameDurMs + stepMs; ms += stepMs)
            {
                float x = origin.x + ms * pxPerMs;
                dl->AddLine(ImVec2(x, rulerY), ImVec2(x, rulerY + rulerH),
                            IM_COL32(140, 140, 140, 200), 1.0f);
                char buf[16];
                snprintf(buf, sizeof(buf), "%.0fms", ms);
                dl->AddText(ImVec2(x + 2, rulerY + 3),
                            IM_COL32(200, 200, 200, 255), buf);
            }
        }

        // Depth labels on the left
        for (int d = 0; d <= maxDepth; ++d)
        {
            float y = origin.y + rulerH + d * rowH;
            char label[32];
            snprintf(label, sizeof(label), "Depth %d", d);
            dl->AddText(ImVec2(origin.x - labelW + 4, y + 4),
                        IM_COL32(180, 180, 180, 255), label);
            // row bg
            uint32_t rowBg = (d % 2 == 0)
                ? IM_COL32(38, 38, 38, 255)
                : IM_COL32(44, 44, 44, 255);
            dl->AddRectFilled(ImVec2(origin.x, y),
                              ImVec2(origin.x + totalW, y + rowH), rowBg);
        }

        // Sample bars
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        for (auto& s : fd.samples)
        {
            float x0 = origin.x + (float)s.startMs * pxPerMs;
            float y0 = origin.y + rulerH + s.depth * rowH + 1;
            float x1 = x0 + std::max(1.0f, (float)s.durationMs * pxPerMs);
            float y1 = y0 + rowH - 2;

            uint32_t col  = s.color;
            bool     hov  = (mousePos.x >= x0 && mousePos.x <= x1 &&
                              mousePos.y >= y0 && mousePos.y <= y1);
            if (hov) col = BrightenColor(col);

            dl->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), col, 2.0f);
            dl->AddRect      (ImVec2(x0, y0), ImVec2(x1, y1),
                              IM_COL32(0, 0, 0, 80), 2.0f);

            // Label inside bar if wide enough
            if (x1 - x0 > 30)
            {
                char label[64];
                snprintf(label, sizeof(label), "%s %.2fms", s.name, s.durationMs);
                dl->AddText(ImVec2(x0 + 3, y0 + 4),
                            IM_COL32(255, 255, 255, 220), label);
            }

            if (hov)
            {
                ImGui::SetTooltip("%s\nStart:    %.3f ms\nDuration: %.3f ms",
                                  s.name, s.startMs, s.durationMs);
            }
        }

        ImGui::EndChild();
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

    // ── Memory ───────────────────────────────────────────────────────

    void RenderMemory(const std::vector<FrameData>& frames)
    {
        const FrameData& last = frames.back();

        // Stats row
        ImGui::Text("Live allocations : %zu",   last.memAllocCount);
        ImGui::SameLine(250);
        ImGui::Text("Heap in use : %.2f KB",    last.memAllocated / 1024.0);

        ImGui::Separator();
        ImGui::TextUnformatted("Heap usage over last frames:");

        // Graph
        const float graphH = 120.0f;
        float       availW = ImGui::GetContentRegionAvail().x;
        ImDrawList* dl     = ImGui::GetWindowDrawList();
        ImVec2      origin = ImGui::GetCursorScreenPos();

        ImGui::InvisibleButton("##mem_graph", ImVec2(availW, graphH));
        dl->AddRectFilled(origin,
                          ImVec2(origin.x + availW, origin.y + graphH),
                          IM_COL32(28, 28, 28, 255));

        // Find max for scale
        size_t maxMem = 1;
        for (auto& f : frames) maxMem = std::max(maxMem, f.memAllocated);

        int   count  = (int)frames.size();
        float stepX  = availW / std::max(count - 1, 1);

        // Draw filled area
        std::vector<ImVec2> pts;
        pts.reserve(count + 2);
        for (int i = 0; i < count; ++i)
        {
            float t = (float)frames[i].memAllocated / (float)maxMem;
            float x = origin.x + i * stepX;
            float y = origin.y + graphH - t * (graphH - 4);
            pts.push_back(ImVec2(x, y));
        }

        // Fill under the line
        for (int i = 0; i + 1 < count; ++i)
        {
            ImVec2 p0 = pts[i], p1 = pts[i + 1];
            dl->AddQuadFilled(
                p0, p1,
                ImVec2(p1.x, origin.y + graphH),
                ImVec2(p0.x, origin.y + graphH),
                IM_COL32(60, 140, 220, 80));
        }

        // Line on top
        for (int i = 0; i + 1 < count; ++i)
            dl->AddLine(pts[i], pts[i + 1], IM_COL32(100, 180, 255, 220), 1.5f);

        // Y-axis labels
        for (int i = 0; i <= 4; ++i)
        {
            float pct = i / 4.0f;
            float y   = origin.y + graphH - pct * (graphH - 4);
            size_t kb = (size_t)(maxMem * pct / 1024);
            char buf[32];
            snprintf(buf, sizeof(buf), "%zu KB", kb);
            dl->AddText(ImVec2(origin.x + 4, y - 10),
                        IM_COL32(160, 160, 160, 200), buf);
            dl->AddLine(ImVec2(origin.x, y),
                        ImVec2(origin.x + availW, y),
                        IM_COL32(60, 60, 60, 120), 1.0f);
        }

        // Hover tooltip
        if (ImGui::IsItemHovered())
        {
            float mx  = ImGui::GetIO().MousePos.x;
            int   idx = (int)((mx - origin.x) / stepX + 0.5f);
            idx = std::clamp(idx, 0, count - 1);
            ImGui::SetTooltip("Frame %d\nAllocs: %zu\nBytes: %.2f KB",
                              idx,
                              frames[idx].memAllocCount,
                              frames[idx].memAllocated / 1024.0);
        }

        ImGui::Dummy(ImVec2(availW, 4));
        ImGui::Separator();

        // Allocation count graph
        ImGui::TextUnformatted("Allocation count over last frames:");
        ImVec2 origin2 = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##alloc_graph", ImVec2(availW, graphH));
        dl->AddRectFilled(origin2,
                          ImVec2(origin2.x + availW, origin2.y + graphH),
                          IM_COL32(28, 28, 28, 255));

        size_t maxAlloc = 1;
        for (auto& f : frames) maxAlloc = std::max(maxAlloc, f.memAllocCount);

        for (int i = 0; i + 1 < count; ++i)
        {
            float t0 = (float)frames[i].memAllocCount   / (float)maxAlloc;
            float t1 = (float)frames[i+1].memAllocCount / (float)maxAlloc;
            float x0 = origin2.x + i * stepX;
            float x1 = origin2.x + (i + 1) * stepX;
            float y0 = origin2.y + graphH - t0 * (graphH - 4);
            float y1 = origin2.y + graphH - t1 * (graphH - 4);
            dl->AddLine(ImVec2(x0, y0), ImVec2(x1, y1),
                        IM_COL32(220, 140, 60, 220), 1.5f);
        }
    }

    // ── Helpers ──────────────────────────────────────────────────────

    static uint32_t BrightenColor(uint32_t col)
    {
        uint8_t r = std::min(255, (int)((col >>  0) & 0xFF) + 50);
        uint8_t g = std::min(255, (int)((col >>  8) & 0xFF) + 50);
        uint8_t b = std::min(255, (int)((col >> 16) & 0xFF) + 50);
        return (col & 0xFF000000) | ((uint32_t)b << 16) | ((uint32_t)g << 8) | r;
    }
};
