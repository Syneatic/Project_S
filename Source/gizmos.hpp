#pragma once

#include "renderer.hpp"
#include "math.hpp"

enum class GizmoAxis { NONE, X, Y, CENTER, ROTATION };
enum class GizmoMode { TRANSLATE, ROTATE, SCALE };


// ===== TRANSFORM GIZMOS =====
inline void DrawTranslationGizmo(float2 pos, float scale = 0.65f) {
    float handleLength = 65.0f * scale;
    float thickness = 25.0f * scale;

    // --- X Axis (Red) ---
    RenderData xArrow;
    xArrow.transform.position = pos;
    xArrow.transform.rotation = 0.0f;
    xArrow.transform.scale = { handleLength, thickness };
    xArrow.color = { 1.0f, 0.0f, 0.0f, 1.0f }; // Red
    xArrow.blendMode = AE_GFX_BM_NONE;
    RenderSystem::DrawArrow(xArrow);

    // --- Y Axis (Green) ---
    RenderData yArrow;
    yArrow.transform.position = pos;
    yArrow.transform.rotation = 90.0f;
    yArrow.transform.scale = { handleLength, thickness };
    yArrow.color = { 0.0f, 1.0f, 0.0f, 1.0f }; // Green
    yArrow.blendMode = AE_GFX_BM_NONE;
    RenderSystem::DrawArrow(yArrow);

    // --- Center Handle (Yellow/White) ---
    RenderData centerBox;
    centerBox.transform.position = pos;
    centerBox.transform.scale = { thickness * 1.f, thickness * 1.f };
    centerBox.color = { 1.0f, 1.0f, 0.0f, 1.0f }; // Yellow
    centerBox.alignment = MC;
    centerBox.blendMode = AE_GFX_BM_NONE;
    RenderSystem::DrawQuad(centerBox);
}

inline void DrawRotationGizmo(float2 pos, float scale = 0.5f) {
    float rotationRadius = 65.0f * scale * 1.2f;

    RenderData ring;
    ring.transform.position = pos;
    ring.transform.scale = { rotationRadius * 2.0f, rotationRadius * 2.0f };
    ring.color = { 0.0f, 1.0f, 1.0f, 1.0f }; // Cyan
    ring.meshMode = AE_GFX_MDM_LINES_STRIP;
    ring.alignment = MC;
    ring.blendMode = AE_GFX_BM_NONE;
    RenderSystem::DrawCircle(ring);
}

inline void DrawScaleGizmo(float2 pos, float scale = 0.65f) {
    float handleLength = 65.0f * scale;
    float thickness = 25.0f * scale;
    float boxSize = thickness * 0.65f;
    // X Scale (Red with Box end)
    RenderData xBar;
    xBar.transform.position = pos;
    xBar.transform.scale = { handleLength, thickness * 0.2f };
    xBar.color = { 1.0f, 0.0f, 0.0f, 1.0f };
    xBar.alignment = ML;
    xBar.blendMode = AE_GFX_BM_NONE;
    RenderSystem::DrawQuad(xBar);

    RenderData xBox;
    xBox.transform.position = { pos.x + handleLength, pos.y };
    xBox.transform.scale = { boxSize, boxSize };
    xBox.color = { 1.0f, 0.0f, 0.0f, 1.0f };
    xBox.alignment = MC;
    xBox.blendMode = AE_GFX_BM_NONE;
    RenderSystem::DrawQuad(xBox);

    // Y Scale (Green with Box end)
    RenderData yBar;
    yBar.transform.position = pos;
    yBar.transform.rotation = 90.0f;
    yBar.transform.scale = { handleLength, thickness * 0.2f };
    yBar.color = { 0.0f, 1.0f, 0.0f, 1.0f };
    yBar.alignment = ML;
    yBar.blendMode = AE_GFX_BM_NONE;
    RenderSystem::DrawQuad(yBar);

    RenderData yBox;
    yBox.transform.position = { pos.x, pos.y + handleLength };
    yBox.transform.scale = { boxSize, boxSize };
    yBox.color = { 0.0f, 1.0f, 0.0f, 1.0f };
    yBox.alignment = MC;
    yBox.blendMode = AE_GFX_BM_NONE;
    RenderSystem::DrawQuad(yBox);

    RenderData centerBox;
    centerBox.transform.position = pos;
    centerBox.transform.rotation = 90.0f;
    centerBox.transform.scale = { boxSize,boxSize };    
    centerBox.color = { 1, 1, 0, 1 };
    centerBox.alignment = MC;
    centerBox.blendMode = AE_GFX_BM_NONE;
    RenderSystem::DrawQuad(centerBox);
}

// Helper for AABB collision
inline bool IsPointInRect(float2 p, float2 pos, float2 size, Alignment align) {
    float left, right, top, bottom;

    // Logic derived from RenderSystem::SetTransform in renderer.cpp
    switch (align) {
    case ML: // Middle-Left: Origin is at the left center
        left = pos.x;
        right = pos.x + size.x;
        top = pos.y + size.y * 0.5f;
        bottom = pos.y - size.y * 0.5f;
        break;
    case MC: // Middle-Center: Origin is exactly in the middle
        left = pos.x - size.x * 0.5f;
        right = pos.x + size.x * 0.5f;
        top = pos.y + size.y * 0.5f;
        bottom = pos.y - size.y * 0.5f;
        break;
    default: // Simplified for gizmos; add other cases if needed
        left = pos.x - size.x * 0.5f;
        right = pos.x + size.x * 0.5f;
        top = pos.y + size.y * 0.5f;
        bottom = pos.y - size.y * 0.5f;
        break;
    }

    return (p.x >= left && p.x <= right && p.y >= bottom && p.y <= top);
}

inline bool IsPointOnRing(float2 p, float2 center, float radius, float thickness)
{
    float distance = sqrtf(powf(p.x - center.x, 2) + powf(p.y - center.y, 2));
    return (distance >= radius - thickness && distance <= radius + thickness);
}

inline GizmoAxis GetHitAxis(float2 mouseWorld, float2 handlePos, float scale = 0.5f) {
    float length = 65.0f * scale;
    float thick = 15.0f * scale;
    float rotationRadius = length * 1.2f;

    // 1. Center Check (Yellow box)
    if (IsPointInRect(mouseWorld, handlePos, { thick * 1.5f, thick * 1.5f }, MC))
        return GizmoAxis::CENTER;

    if (IsPointOnRing(mouseWorld, handlePos, rotationRadius, thick * 0.75f))
        return GizmoAxis::ROTATION;

    // 2. X Axis Check (Red Arrow)
    if (IsPointInRect(mouseWorld, handlePos, { length, thick }, ML))
        return GizmoAxis::X;

    // 3. Y Axis Check (Green Arrow)
    // For Y, we treat it as a vertical rectangle growing upwards from handlePos
    float yLeft = handlePos.x - thick * 0.5f;
    float yRight = handlePos.x + thick * 0.5f;
    float yBottom = handlePos.y;
    float yTop = handlePos.y + length;

    if (mouseWorld.x >= yLeft && mouseWorld.x <= yRight &&
        mouseWorld.y >= yBottom && mouseWorld.y <= yTop)
        return GizmoAxis::Y;

    return GizmoAxis::NONE;
}
// ===== TRANSFORM GIZMOS =====



inline void DrawBox(float2 pos, float2 size, Color color)
{
    RenderData rd;
    rd.transform.position = pos;
    rd.transform.scale = size;
    rd.color = color;
    rd.meshMode = AE_GFX_MDM_LINES; // Draw as wireframe
    RenderSystem::DrawQuad(rd);
}
