#pragma once
#include <vector>
#include <unordered_set>

#include "AEEngine.h"
#include "component.hpp"
#include "color.hpp"

struct Renderer;

struct RenderData //pass in this data to Draw functions
{
	Transform transform{};

	AEGfxBlendMode blendMode{};
	AEGfxRenderMode renderMode{};
	Color color{};
	AEGfxTexture* texture = nullptr;
	//draw mode maybe
};


typedef enum drawMode {
	center,
	lSide
}drawMode;

namespace renderSys {
	void rendererInit();

	void drawRect(float2 pos, float rotAngle, float2 size, drawMode alignment);
	void drawTri(float2 pos, float angle, float size);
	void drawCirc(float2 pos, float angle, float size);
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