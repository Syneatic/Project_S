#pragma once
#include <string>
#include <iostream>
#include "ImGUI/imgui.h"

#include "AEEngine.h"
#include "gameobject.hpp"
#include "color.hpp"

enum RenderLayer
{
    BACKGROUND = 0,
    DEFAULT = 500,
    UI = 999
};

//abstract
struct Renderer : Component
{
    AEGfxBlendMode blendMode{};
    AEGfxRenderMode renderMode{};
    AEGfxMeshDrawMode meshDrawMode{};
    RenderLayer renderLayer{};
    Color color{};
    AEGfxTexture* texture = nullptr;

    void DrawInInspector() override
    {
        static const char* _blendNames[] = { "NONE", "BLEND", "ADD", "MULTIPLY", "NUM"};
        static const char* _renderNames[] = { "NONE", "COLOR", "TEXTURE", "NUM"};
        static const char* _meshDrawNames[] = { "POINTS", "LINES", "LINES_STRIP", "TRIS","NUM"};


        if (ImGui::BeginCombo("Blend Mode", _blendNames[(int)blendMode]))
        {
            for (int i = 0; i < 5; ++i)
            {
                bool selected = (i == blendMode);
                if (ImGui::Selectable(_blendNames[i], selected))
                {
                    blendMode = (AEGfxBlendMode)i;
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        if (ImGui::BeginCombo("Render Mode", _renderNames[(int)renderMode]))
        {
            for (int i = 0; i < 4; ++i)
            {
                bool selected = (i == renderMode);
                if (ImGui::Selectable(_renderNames[i], selected))
                {
                    renderMode = (AEGfxRenderMode)i;
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        if (ImGui::BeginCombo("Mesh Draw Mode", _meshDrawNames[(int)meshDrawMode]))
        {
            for (int i = 0; i < 5; ++i)
            {
                bool selected = (i == meshDrawMode);
                if (ImGui::Selectable(_meshDrawNames[i], selected))
                {
                    meshDrawMode = (AEGfxMeshDrawMode)i;
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();

        if (ImGui::BeginCombo("Mesh Draw Mode", _meshDrawNames[(int)meshDrawMode]))
        {
            for (int i = 0; i < 5; ++i)
            {
                bool selected = (i == meshDrawMode);
                if (ImGui::Selectable(_meshDrawNames[i], selected))
                {
                    meshDrawMode = (AEGfxMeshDrawMode)i;
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::TextUnformatted("Color");
        float col[4] = { color.r, color.g, color.b, color.a };
        if (ImGui::ColorEdit4("##renderer_color", col))
        {
            color.r = col[0];
            color.g = col[1];
            color.b = col[2];
            color.a = col[3];
        }
    }

    virtual void Draw()
    {

    }
};

struct SpriteRenderer : Renderer
{
    void Draw() override
    {
        GameObject& owner = *_owner;
        Transform& trans = *owner.GetComponent<Transform>();
        //put texture on
        //draw quad
        renderSys::DrawRect(trans.position, trans.rotation, trans.scale, center);
    }
    const std::string name() const override { return "SpriteRenderer"; }
};

struct MeshRenderer : Renderer
{
    AEGfxVertexBuffer* mesh = nullptr;
    const std::string name() const override { return "MeshRenderer"; }
};