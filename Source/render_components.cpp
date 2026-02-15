#include <string>
#include <iostream>
#include "ImGUI/imgui.h"
#include "json_parser_helper.hpp"

#include "render_components.hpp"
#include "gameobject.hpp"

// ===== RENDERER DEF =====
void Renderer::DrawInInspector()
{
    static const char* _blendNames[] = { "NONE", "BLEND", "ADD", "MULTIPLY"};
    static const char* _renderNames[] = { "NONE", "COLOR", "TEXTURE"};
    static const char* _meshDrawNames[] = { "POINTS", "LINES", "LINES_STRIP", "TRIS"};
    static const char* _alignmentNames[] = { "Top left", "Top", "Top right", "Left", "Center", "Right", "Bottom Left", "Bottom", "Bottom right"};

    if (ImGui::BeginCombo("Blend Mode", _blendNames[(int)blendMode]))
    {
        for (int i = 0; i < static_cast<int>(AE_GFX_BM_NUM); ++i)
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
        for (int i = 0; i < static_cast<int>(AE_GFX_RM_NUM); ++i)
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
        for (int i = 0; i < static_cast<int>(AE_GFX_MDM_NUM); ++i)
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

void Renderer::Serialize(Json::Value& outComp) const
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

void Renderer::Deserialize(const Json::Value& compObj)
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

void Renderer::Draw()
{

}
// ===== RENDERER DEF =====




void SpriteRenderer::Draw()
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




void MeshRenderer::Serialize(Json::Value& outComp) const
{
    Renderer::Serialize(outComp);
    //save mesh here somehow
}

void MeshRenderer::Deserialize(const Json::Value& compObj)
{
    Renderer::Deserialize(compObj);
    //load mesh here
}



// ===== TEXT_RENDERER DEF =====
void TextRenderer::DrawInInspector()
{
    static const char* _alignmentNames[] = { "Top left", "Top", "Top right", "Left", "Center", "Right", "Bottom Left", "Bottom", "Bottom right" };

    char textBuffer[256];
    strcpy_s(textBuffer, myText.c_str());
    if (ImGui::InputText("##textrenderer", textBuffer, sizeof(textBuffer)))
    {
        myText = textBuffer;
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

void TextRenderer::Draw()
{
    GameObject& owner = *_owner;
    RenderData data{};
    data.transform = *owner.GetComponent<Transform>();
    data.renderLayer = renderLayer;
    data.color = color;
    data.alignment = alignment;
    RenderSystem::DrawMyText(myText.c_str(), data);
}

void TextRenderer::Serialize(Json::Value& outComp) const
{
    outComp["renderlayer"] = renderLayer;
    outComp["alignment"] = alignment;
    outComp["color"] = WriteColor(color);
    outComp["text"] = myText;

    //texture is abit tricky for now
    //i think save it as a filename for now
}

void TextRenderer::Deserialize(const Json::Value& compObj)
{
    if (compObj.isMember("renderlayer") && compObj["renderlayer"].isInt())
        renderLayer = static_cast<RenderLayer>(compObj["renderlayer"].asInt());

    if (compObj.isMember("alignment") && compObj["alignment"].isInt())
        alignment = static_cast<Alignment>(compObj["alignment"].asInt());

    if (compObj.isMember("color"))
        ReadColor(compObj["color"], color);

    if (compObj.isMember("text") || !compObj["text"].isString()) {
        myText = compObj["text"].asString();
    }

    //read texture from file here
}
// ===== TEXT_RENDERER DEF =====
