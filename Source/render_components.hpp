#pragma once
#include <string>
#include <iostream>
#include "ImGUI/imgui.h"

#include "AEEngine.h"
#include "gameobject.hpp"
#include "color.hpp"
#include "renderer.hpp"
//#include "json_parser_helper.hpp"

//abstract
struct Renderer : Component
{
    AEGfxBlendMode blendMode{ AE_GFX_BM_BLEND };
    AEGfxRenderMode renderMode{ AE_GFX_RM_COLOR};
    AEGfxMeshDrawMode meshDrawMode{ AE_GFX_MDM_TRIANGLES};
    RenderLayer renderLayer{ DEFAULT };
    Alignment alignment{MC};
    Color color{1.f,1.f,1.f,1.f};
    AEGfxTexture* texture = nullptr;

    void DrawInInspector() override
    {
        static const char* _blendNames[] = { "NONE", "BLEND", "ADD", "MULTIPLY", "NUM"};
        static const char* _renderNames[] = { "NONE", "COLOR", "TEXTURE", "NUM"};
        static const char* _meshDrawNames[] = { "POINTS", "LINES", "LINES_STRIP", "TRIS","NUM"};
        static const char* _alignmentNames[] = { "Top left", "Top", "Top right", "Left", "Center", "Right", "Bottom Left", "Bottom", "Bottom right"};


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

        if (ImGui::BeginCombo("Alignment", _alignmentNames[(int)alignment]))
        {
            for (int i = 0; i < 9; ++i)
            {
                bool selected = (i == alignment);
                if (ImGui::Selectable(_alignmentNames[i], selected))
                {
                    alignment = (Alignment)i;
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

    void Serialize(Json::Value& outComp) const override
    {
        outComp["blendmode"] = blendMode;
        outComp["rendermode"] = renderMode;
        outComp["meshdrawmode"] = meshDrawMode;
        outComp["renderlayer"] = renderLayer;
        outComp["alignment"] = alignment;
        outComp["color"] = WriteColor(color);

        //texture is abit tricky for now
        //i think save it as a filename for now
    }

    void Deserialize(const Json::Value& compObj) override
    {
        if (compObj.isMember("blendmode") && compObj["blendmode"].isInt())
            blendMode = static_cast<AEGfxBlendMode>(compObj["blendmode"].asInt());
        
        if (compObj.isMember("rendermode") && compObj["rendermode"].isInt())
            renderMode = static_cast<AEGfxRenderMode>(compObj["rendermode"].asInt());

        if (compObj.isMember("meshdrawmode") && compObj["meshdrawmode"].isInt())
            meshDrawMode = static_cast<AEGfxMeshDrawMode>(compObj["meshdrawmode"].asInt());

        if (compObj.isMember("renderlayer") && compObj["renderlayer"].isInt())
            renderLayer = static_cast<RenderLayer>(compObj["renderlayer"].asInt());

        if (compObj.isMember("alignment") && compObj["alignment"].isInt())
            alignment = static_cast<Alignment>(compObj["alignment"].asInt());

        if (compObj.isMember("color"))
            ReadColor(compObj["color"],color);

        //read texture from file here
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
        RenderData data{};
        data.transform = *owner.GetComponent<Transform>();
        data.blendMode = blendMode;
        data.renderMode = renderMode;
        data.meshMode = meshDrawMode;
        data.renderLayer = renderLayer;
        data.color = color;
        data.texture = texture;
        data.alignment = alignment;
        RenderSystem::DrawQuad(data);
    }

    //no override since sprite is quite normal

    const std::string name() const override { return "SpriteRenderer"; }
};

struct MeshRenderer : Renderer
{
    AEGfxVertexBuffer* mesh = nullptr;

    void Serialize(Json::Value& outComp) const override
    {
        Renderer::Serialize(outComp);
        //save mesh here somehow
    }

    void Deserialize(const Json::Value& compObj) override
    {
        Renderer::Deserialize(compObj);
        //load mesh here
    }

    const std::string name() const override { return "MeshRenderer"; }
};