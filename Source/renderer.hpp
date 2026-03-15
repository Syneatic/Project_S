#pragma once

namespace Graphics
{
	using RenderMode = AEGfxRenderMode;
	using BlendMode = AEGfxBlendMode;
	using DrawMode = AEGfxMeshDrawMode;
	using MTX = AEMtx33;
	using VertexBuffer = AEGfxVertexList;
	using Texture = AEGfxTexture;
	using Font = s8;

    enum RenderLayer
    {
        BACKGROUND = 0,
        DEFAULT,
        UI,
        GIZMOS,
    };

    enum class PrimitiveType
    {
        QUAD,
        TRIANGLE,
        CIRCLE,
        BOX, // Gizmo
        TEXT,
    };

    enum class Alignment
    {
        TL, TC, TR,
        ML, MC, MR,
        BL, BC, BR
    };

    struct RenderData
    {
        //spatial
        float2 pos{};
        float2 scale{};
        f32 rot{};
        Alignment alignment{ Alignment::MC };
        bool isScreenSpace = false;

        //sorting
        RenderLayer layer{ DEFAULT };
        f32 sortOrder{ 0.f };

        //visuals
        Texture* texture = nullptr;
        Color color{};

        //AE
        BlendMode blendMode{ AE_GFX_BM_BLEND };
        RenderMode renderMode{ AE_GFX_RM_COLOR };
        DrawMode drawMode{ AE_GFX_MDM_TRIANGLES };

        u64 sortKey{ 0 };
    };

	struct RenderCommand;

    VertexBuffer* QuadMesh();

    Texture* LoadTexture(std::string fileName);

	void Initialize();
    void Flush();//flushes all commands
	void Exit();

	void Submit(const RenderData& data, PrimitiveType type, const char* text = nullptr);
    void SubmitArrow(const RenderData& data); //workaround

	void Execute();
}