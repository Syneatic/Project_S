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

typedef enum DrawMode {
	TL, TC, TR,
	ML, MC, MR,
	BL, BC, BR
}DrawMode;

struct RenderData //pass in this data to Draw functions
{
	Transform transform{};

	AEGfxBlendMode blendMode{};
	AEGfxRenderMode renderMode{};
	AEGfxMeshDrawMode meshMode{};
	RenderLayer renderLayer{};
	Color color{};
	AEGfxTexture* texture = nullptr;
	DrawMode drawmode{};
};

namespace renderSys {
	void rendererInit();

	void DrawRect(RenderData data);
	void DrawTri(RenderData data);
	void DrawCirc(RenderData data);
	void DrawMyText(char* text, float2 pos, float size);
	void DrawArrow(float2 pos);
	void rendererExit();
}

struct RenderSystem
{
	void RegisterRenderer(Renderer* r);
	void UnregisterRenderer(Renderer* r);
	void FlushRenderers();
	void Draw();

	static RenderSystem& Instance();

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;
	RenderSystem(RenderSystem&&) = delete;
	RenderSystem& operator=(RenderSystem&&) = delete;

private:
	std::vector<Renderer*> _renderers;
	std::unordered_set<Renderer*> _rendererSet;

	RenderSystem() = default;
	~RenderSystem() = default;
};