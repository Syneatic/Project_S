#pragma once
#include <vector>
#include <unordered_set>

#include "AEEngine.h"
#include "component.hpp"
#include "color.hpp"

struct Renderer;

typedef enum DrawMode {
	center,
	lSide
}DrawMode;

struct RenderData //pass in this data to Draw functions
{
	Transform transform{};

	AEGfxBlendMode blendMode{};
	AEGfxRenderMode renderMode{};
	Color color{};
	AEGfxTexture* texture = nullptr;
	DrawMode drawmode{};
};

namespace renderSys {
	void rendererInit();

	void DrawRect(Transform transform, DrawMode alignment);
	void DrawTri(Transform transform);
	void DrawCirc(Transform transform);
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