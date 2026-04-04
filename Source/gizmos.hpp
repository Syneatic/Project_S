/*
Author: Yan Chun
Co-Author: Nil
*/
#pragma once

#include "renderer.hpp"
#include "math.hpp"

enum class GizmoAxis { NONE, X, Y, CENTER, ROTATION };
enum class GizmoMode { TRANSLATE, ROTATE, SCALE };

static Graphics::RenderLayer editorTransformLayer = (Graphics::RenderLayer)(Graphics::RenderLayer::GIZMOS+50);

// ===== TRANSFORM GIZMOS =====
inline void DrawTranslationGizmo(float2 pos, float scale = 0.65f) 
{
    float handleLength = 65.0f * scale;
    float thickness = 25.0f * scale;

    // --- X Axis (Red) ---
    Graphics::RenderData xArrow;
    xArrow.pos = pos;
    xArrow.rot = 0.0f;
    xArrow.scale = { handleLength, thickness };
    xArrow.layer = editorTransformLayer;
    xArrow.color = { 1.0f, 0.0f, 0.0f, 1.0f }; // Red
    xArrow.blendMode = AE_GFX_BM_NONE;
    xArrow.drawMode = AE_GFX_MDM_TRIANGLES;
    Graphics::SubmitArrow(xArrow);

    // --- Y Axis (Green) ---
    Graphics::RenderData yArrow;
    yArrow.pos = pos;
    yArrow.rot = 90.0f;
    yArrow.scale = { handleLength, thickness };
    yArrow.layer = editorTransformLayer;
    yArrow.color = { 0.0f, 1.0f, 0.0f, 1.0f }; // Green
    yArrow.blendMode = AE_GFX_BM_NONE;
    yArrow.drawMode = AE_GFX_MDM_TRIANGLES;
    Graphics::SubmitArrow(yArrow);

    // --- Center Handle (Yellow/White) ---
    Graphics::RenderData centerBox;
    centerBox.pos = pos;
    centerBox.scale = { thickness * 1.f, thickness * 1.f };
    centerBox.layer = editorTransformLayer;
    centerBox.color = { 1.0f, 1.0f, 0.0f, 1.0f }; // Yellow
    centerBox.alignment = Graphics::Alignment::MC;
    centerBox.blendMode = AE_GFX_BM_NONE;
    centerBox.drawMode = AE_GFX_MDM_TRIANGLES;
    Graphics::Submit(centerBox,Graphics::PrimitiveType::QUAD);
}

inline void DrawRotationGizmo(float2 pos, float scale = 0.5f) {
    float rotationRadius = 65.0f * scale * 1.2f;

    Graphics::RenderData ring;
    ring.pos = pos;
    ring.scale = { rotationRadius * 2.0f, rotationRadius * 2.0f };
    ring.color = { 0.0f, 1.0f, 1.0f, 1.0f }; // Cyan
    ring.drawMode = AE_GFX_MDM_LINES_STRIP;
    ring.alignment = Graphics::Alignment::MC;
    ring.blendMode = AE_GFX_BM_NONE;
    ring.layer = editorTransformLayer;
    Graphics::Submit(ring, Graphics::PrimitiveType::CIRCLE);
    //RenderSystem::DrawCircle(ring);
}

inline void DrawScaleGizmo(float2 pos, float scale = 0.65f) {
    float handleLength = 65.0f * scale;
    float thickness = 25.0f * scale;
    float boxSize = thickness * 0.65f;

    // X Scale (Red with Box end)
    Graphics::RenderData xBar;
    xBar.pos = pos;
    xBar.scale = { handleLength, thickness * 0.2f };
    xBar.color = { 1.0f, 0.0f, 0.0f, 1.0f };
    xBar.alignment = Graphics::Alignment::ML;
    xBar.blendMode = AE_GFX_BM_NONE;
    xBar.layer = editorTransformLayer;
    Graphics::Submit(xBar,Graphics::PrimitiveType::QUAD);

    Graphics::RenderData xBox;
    xBox.pos = { pos.x + handleLength, pos.y };
    xBox.scale = { boxSize, boxSize };
    xBox.color = { 1.0f, 0.0f, 0.0f, 1.0f };
    xBox.alignment = Graphics::Alignment::MC;
    xBox.blendMode = AE_GFX_BM_NONE;
    xBox.layer = editorTransformLayer;
    Graphics::Submit(xBox,Graphics::PrimitiveType::QUAD);

    // Y Scale (Green with Box end)
    Graphics::RenderData yBar;
    yBar.pos = pos;
    yBar.rot = 90.0f;
    yBar.scale = { handleLength, thickness * 0.2f };
    yBar.color = { 0.0f, 1.0f, 0.0f, 1.0f };
    yBar.alignment = Graphics::Alignment::ML;
    yBar.blendMode = AE_GFX_BM_NONE;
    yBar.layer = editorTransformLayer;
    Graphics::Submit(yBar,Graphics::PrimitiveType::QUAD);

    Graphics::RenderData yBox;
    yBox.pos = { pos.x, pos.y + handleLength };
    yBox.scale = { boxSize, boxSize };
    yBox.color = { 0.0f, 1.0f, 0.0f, 1.0f };
    yBox.alignment = Graphics::Alignment::MC;
    yBox.blendMode = AE_GFX_BM_NONE;
    yBox.layer = editorTransformLayer;
    Graphics::Submit(yBox,Graphics::PrimitiveType::QUAD);

    Graphics::RenderData centerBox;
    centerBox.pos = pos;
    centerBox.rot = 90.0f;
    centerBox.scale = { thickness * 1.f, thickness * 1.f };
    centerBox.color = { 1, 1, 0, 1 };
    centerBox.alignment = Graphics::Alignment::MC;
    centerBox.blendMode = AE_GFX_BM_NONE;
    centerBox.layer = editorTransformLayer;
    Graphics::Submit(centerBox,Graphics::PrimitiveType::QUAD);
}

// Helper for AABB collision
inline bool IsPointInRect(float2 p, float2 pos, float2 size, Graphics::Alignment align) 
{
    float left, right, top, bottom;

    // Logic derived from RenderSystem::SetTransform in renderer.cpp
    switch (align) {
    case Graphics::Alignment::ML: // Middle-Left: Origin is at the left center
        left = pos.x;
        right = pos.x + size.x;
        top = pos.y + size.y * 0.5f;
        bottom = pos.y - size.y * 0.5f;
        break;
    case Graphics::Alignment::MC: // Middle-Center: Origin is exactly in the middle
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
    if (IsPointInRect(mouseWorld, handlePos, { thick * 1.5f, thick * 1.5f }, Graphics::Alignment::MC))
        return GizmoAxis::CENTER;

    if (IsPointOnRing(mouseWorld, handlePos, rotationRadius, thick * 0.75f))
        return GizmoAxis::ROTATION;

    // 2. X Axis Check (Red Arrow)
    if (IsPointInRect(mouseWorld, handlePos, { length, thick }, Graphics::Alignment::ML))
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


