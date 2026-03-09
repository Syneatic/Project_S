#include "render_components.hpp"
#include "gameobject.hpp"
#include <iostream>

namespace
{
    std::wstring OpenFilePng()
    {
        namespace fs = std::filesystem;
        std::wstring targetDir = L"../../Assets/";

        try {
            if (fs::exists(targetDir)) {
                targetDir = fs::absolute(targetDir).wstring();
            }
        }
        catch (...) {}

        HRESULT hrInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        bool didCoInit = SUCCEEDED(hrInit) || hrInit == RPC_E_CHANGED_MODE;

        IFileOpenDialog* dialog = nullptr;
        if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr,
            CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))))
        {
            if (didCoInit && hrInit != RPC_E_CHANGED_MODE)
                CoUninitialize();
            return L"";
        }

        dialog->SetTitle(L"Select Image");

        COMDLG_FILTERSPEC filters[] = {
            { L"PNG files (*.png)", L"*.png" },
            { L"All files (*.*)",       L"*.*" }

        };
        dialog->SetFileTypes((UINT)std::size(filters), filters);
        dialog->SetFileTypeIndex(1);

        IShellItem* startFolder = nullptr;
        if (SUCCEEDED(SHCreateItemFromParsingName(targetDir.c_str(), nullptr, IID_PPV_ARGS(&startFolder))))
        {
            dialog->SetFolder(startFolder);
            dialog->SetDefaultFolder(startFolder);
            startFolder->Release();
        }

        std::wstring result;

        if (SUCCEEDED(dialog->Show(nullptr)))
        {
            IShellItem* item = nullptr;
            if (SUCCEEDED(dialog->GetResult(&item)))
            {
                PWSTR path = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path)))
                {
                    result = path;
                    CoTaskMemFree(path);
                }
                item->Release();
            }
        }

        dialog->Release();
        if (didCoInit && hrInit != RPC_E_CHANGED_MODE)
            CoUninitialize();

        return result;
    }
}

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


//Graphics::RenderLayer renderLayer{ Graphics::RenderLayer::DEFAULT };
//f32 sortOrder{ 0.f };
//bool isScreenSpace{ false };

void SpriteRenderer::DrawInInspector()
{
    static const char* _blendNames[] = { "NONE", "BLEND", "ADD", "MULTIPLY" };
    static const char* _renderNames[] = { "NONE", "COLOR", "TEXTURE" };
    static const char* _meshDrawNames[] = { "POINTS", "LINES", "LINES_STRIP", "TRIS" };
    static const char* _alignmentNames[] = { "Top left", "Top", "Top right", "Left", "Center", "Right", "Bottom Left", "Bottom", "Bottom right" };


    ImGui::TextUnformatted("Texture");

    ImGui::BeginDisabled();
    ImGui::TextUnformatted(fileName.empty() ? "NO FILE SELECTED" : fileName.c_str());
    ImGui::EndDisabled();

    ImGui::SameLine();

    if (ImGui::Button("Select##Texture"))
    {
        std::wstring path = OpenFilePng(); // file dialog
        fileName = std::filesystem::path(path).filename().string();
    }

    ImGui::Separator();

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

    //renderlayer
    //sort order

    ImGui::TextUnformatted("IsScreenSpace");
    ImGui::Checkbox("##isscreenspace", &isScreenSpace);
}

void SpriteRenderer::Serialize(Json::Value& outComp) const
{
    outComp["fileName"] = fileName;
    outComp["blendmode"] = blendMode;
    outComp["rendermode"] = renderMode;
    outComp["meshdrawmode"] = meshDrawMode;
    outComp["renderlayer"] = renderLayer;
    outComp["alignment"] = static_cast<int>(alignment);
    outComp["color"] = WriteColor(color);
    outComp["screenspace"] = isScreenSpace;
}

void SpriteRenderer::Deserialize(const Json::Value& compObj)
{
    if (compObj.isMember("fileName") && compObj["fileName"].isString())
        fileName = compObj["fileName"].asString();

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
        ReadColor(compObj["color"], color);

    if (compObj.isMember("screenspace") && compObj["screenspace"].isBool())
        isScreenSpace = compObj["screenspace"].asBool();
}

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
    data.texture = Graphics::LoadTexture(fileName);
    data.color = color;
    // std::cout << fileName.c_str() << "\n";
    
    Graphics::Submit(data,Graphics::PrimitiveType::QUAD);
}

void SpriteRenderer::OnDestroy()
{
    Graphics::RenderData data{};
}

void SpriteRenderer::CopyFrom(Component* src)
{
    auto s = dynamic_cast<SpriteRenderer*>(src);
    if (!s) return;

    fileName = s->fileName;
    blendMode = s->blendMode;
    renderMode = s->renderMode;
    meshDrawMode = s->meshDrawMode;
    renderLayer = s->renderLayer;
    alignment = s->alignment;
    color = s->color;
    sortOrder = s->sortOrder;
    isScreenSpace = s->isScreenSpace;
}

std::unique_ptr<Component> SpriteRenderer::Clone(GameObject& go)
{
    auto n = std::make_unique<SpriteRenderer>(go);
    n.get()->CopyFrom(this);
    return n;
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

    ImGui::TextUnformatted("IsScreenSpace");
    ImGui::Checkbox("##isscreenspace", &isScreenSpace);
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
    outComp["screenspace"] = isScreenSpace;

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

    if (compObj.isMember("screenspace") && compObj["screenspace"].isBool())
        isScreenSpace = compObj["screenspace"].asBool();

    //read texture from file here
}

void TextRenderer::CopyFrom(Component* src)
{
    auto s = dynamic_cast<TextRenderer*>(src);
    if (!s) return;

    fileName = s->fileName;
    blendMode = s->blendMode;
    renderMode = s->renderMode;
    meshDrawMode = s->meshDrawMode;
    renderLayer = s->renderLayer;
    alignment = s->alignment;
    color = s->color;
    sortOrder = s->sortOrder;
    isScreenSpace = s->isScreenSpace;
    text = s->text;
}

std::unique_ptr<Component> TextRenderer::Clone(GameObject& go)
{
    auto n = std::make_unique<TextRenderer>(go);
    n.get()->CopyFrom(this);
    return n;
}

// ===== TEXT_RENDERER DEF =====
