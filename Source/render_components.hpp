#pragma once
#include "renderer.hpp"

//abstract
class Renderer : public Component
{
public:
    std::string fileName{};
    Graphics::BlendMode blendMode{ AE_GFX_BM_BLEND };
    Graphics::RenderMode renderMode{ AE_GFX_RM_COLOR};
    Graphics::DrawMode meshDrawMode{ AE_GFX_MDM_TRIANGLES};
    Graphics::RenderLayer renderLayer{ Graphics::RenderLayer::DEFAULT };
    Graphics::Alignment alignment{Graphics::Alignment::MC};
    Color color{1.f,1.f,1.f,1.f};
    Graphics::Texture* texture = nullptr;
    f32 sortOrder{ 0.f };
    bool isScreenSpace{ false };

    void DrawInInspector() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;
    virtual void Draw();

    void OnStart() override {};

    void OnUpdate() override
    {
        Draw();
    }

    void OnDestroy() override {};

    Renderer(GameObject& go) : Component(go) {};
};

class SpriteRenderer : public Renderer
{
public:
    void DrawInInspector() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;
    void Draw() override;
    void OnDestroy() override;

    const std::string name() const override { return "SpriteRenderer"; }

    SpriteRenderer(GameObject& go) : Renderer(go) {};
    void CopyFrom(Component* src) override;
    std::unique_ptr<Component> Clone(GameObject& go) override;
};

class MeshRenderer : public Renderer
{
public:
    Graphics::VertexBuffer* mesh = nullptr;

    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;

    const std::string name() const override { return "MeshRenderer"; }

    MeshRenderer(GameObject& go) : Renderer(go) {};
    void CopyFrom(Component* src) override;
    std::unique_ptr<Component> Clone(GameObject& go) override;
};

class TextRenderer : public Renderer
{
public:
    std::string text{};

    void DrawInInspector() override;
    void Draw() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;

    const std::string name() const override { return "TextRenderer"; }

    TextRenderer(GameObject& go) : Renderer(go) {};
    void CopyFrom(Component* src) override;
    std::unique_ptr<Component> Clone(GameObject& go) override;
};