#pragma once
#include <vector>
#include <unordered_set>

#include "AEEngine.h"
#include "component.hpp"
#include "color.hpp"

struct Renderer;

enum RenderLayer
{
	BACKGROUND = 0,
	DEFAULT = 500,
	UI = 999
};

typedef enum Alignment {
	TL, TC, TR,
	ML, MC, MR,
	BL, BC, BR
}DrawMode;

struct RenderData //pass in this data to Draw functions
{
	Transform transform{};
	RenderLayer renderLayer{};

	//modes
	AEGfxBlendMode blendMode{AE_GFX_BM_ADD};
	AEGfxRenderMode renderMode{AE_GFX_RM_COLOR};
	AEGfxMeshDrawMode meshMode{AE_GFX_MDM_TRIANGLES};
	Alignment alignment{};

	Color color{};
	AEGfxTexture* texture = nullptr;
};

namespace RenderSystem {
	void RendererInitialize();

	//exposed api for registering renderer when loading levels
	void RegisterRenderer(Renderer* r);
	void UnregisterRenderer(Renderer* r);
	void FlushRenderers();
	void Draw();

	//exposed api for drawing primitives
	void DrawQuad(RenderData data);
	void DrawTri(RenderData data);
	void DrawCirc(RenderData data);
	void DrawPoint(float2 pos);
	void DrawMyText(char* text, RenderData data);
	void DrawArrow(float2 pos);

	void RendererExit();
}