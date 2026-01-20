#pragma once
#include <vector>
#include <unordered_set>

#include "render_components.hpp"

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

//should be singleton as well
struct RenderSystem
{
public:
	void RegisterRenderer(Renderer* r)
	{
		if (!r) return; //check null

		if (_rendererSet.insert(r).second) //insert if not alrdy in set
			_renderers.push_back(r); //only insert to list if ^
	}

	void UnregisterRenderer(Renderer* r)
	{
		if (!r) return;
		if (_rendererSet.erase(r) == 0) return;

		//find the renderer
		auto it = std::find(_renderers.begin(), _renderers.end(), r);
		if (it != _renderers.end())
		{
			//push to back and pop
			*it = _renderers.back();
			_renderers.pop_back();
		}
	}

	void FlushRenderers()
	{
		_renderers.clear();
		_rendererSet.clear();
	}

	void Draw()
	{
		for (auto r : _renderers)
		{
			r->Draw();
		}
	}

private:
	std::vector<Renderer*> _renderers; //iterate through this
	std::unordered_set<Renderer*> _rendererSet; //use set to prevent duplicates

public:
	//make it a singleton
	static RenderSystem& Instance()
	{
		static RenderSystem sys;
		return sys;
	}

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;
	RenderSystem(RenderSystem&&) = delete;
	RenderSystem& operator=(RenderSystem&&) = delete;

	RenderSystem() = default;
	~RenderSystem() = default;
};