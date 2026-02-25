#pragma once
#include "renderer.hpp"

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

    void DrawInInspector() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;
    virtual void Draw();
};

struct SpriteRenderer : Renderer
{
    void Draw() override;

    //no override since sprite is quite normal

    const std::string name() const override { return "SpriteRenderer"; }
};

struct MeshRenderer : Renderer
{
    AEGfxVertexBuffer* mesh = nullptr;

    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;

    const std::string name() const override { return "MeshRenderer"; }
};

struct TextRenderer : Renderer
{
    std::string myText{};

    void DrawInInspector() override;
    void Draw() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;

    const std::string name() const override { return "TextRenderer"; }
};