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
            bool selected = (i == static_cast<int>(alignment));
            if (ImGui::Selectable(_alignmentNames[i], selected))
            {
                alignment = (Graphics::Alignment)i;
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
    outComp["alignment"] = static_cast<int>(alignment);
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
        renderLayer = static_cast<Graphics::RenderLayer>(compObj["renderlayer"].asInt());

    if (compObj.isMember("alignment") && compObj["alignment"].isInt())
        alignment = static_cast<Graphics::Alignment>(compObj["alignment"].asInt());

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
    Transform& t = _transform;
    Graphics::RenderData data{};
    //transform
    data.pos = t.position;
    data.scale = t.scale;
    data.rot = t.rotation;
    data.isScreenSpace = isScreenSpace;
    //states
    data.blendMode = blendMode;
    data.renderMode = renderMode;
    data.drawMode = meshDrawMode;
    data.alignment = alignment;
    data.layer = renderLayer;
    data.sortOrder = sortOrder;

    data.color = color;
    data.texture = texture;
    
    Graphics::Submit(data,Graphics::PrimitiveType::QUAD);
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
    strcpy_s(textBuffer, text.c_str());
    if (ImGui::InputText("##textrenderer", textBuffer, sizeof(textBuffer)))
    {
        text = textBuffer;
    }

    ImGui::Separator();

    if (ImGui::BeginCombo("Alignment", _alignmentNames[(int)alignment]))
    {
        for (int i = 0; i < 9; ++i)
        {
            bool selected = (i == static_cast<int>(alignment));
            if (ImGui::Selectable(_alignmentNames[i], selected))
            {
                alignment = (Graphics::Alignment)i;
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
    Transform& t = _transform;
    Graphics::RenderData data{};
    data.pos = t.position;
    data.scale = t.scale;
    data.rot = t.rotation;
    data.isScreenSpace = isScreenSpace;

    data.alignment = alignment;
    data.layer = renderLayer;
    data.sortOrder = sortOrder;

    data.color = color;
    
    Graphics::Submit(data,Graphics::PrimitiveType::TEXT,text.c_str());
}

void TextRenderer::Serialize(Json::Value& outComp) const
{
    outComp["renderlayer"] = renderLayer;
    outComp["alignment"] = static_cast<int>(alignment);
    outComp["color"] = WriteColor(color);
    outComp["text"] = text;

    //texture is abit tricky for now
    //i think save it as a filename for now
}

void TextRenderer::Deserialize(const Json::Value& compObj)
{
    if (compObj.isMember("renderlayer") && compObj["renderlayer"].isInt())
        renderLayer = static_cast<Graphics::RenderLayer>(compObj["renderlayer"].asInt());

    if (compObj.isMember("alignment") && compObj["alignment"].isInt())
        alignment = static_cast<Graphics::Alignment>(compObj["alignment"].asInt());

    if (compObj.isMember("color"))
        ReadColor(compObj["color"], color);

    if (compObj.isMember("text") || !compObj["text"].isString()) {
        text = compObj["text"].asString();
    }

    //read texture from file here
}
// ===== TEXT_RENDERER DEF =====
