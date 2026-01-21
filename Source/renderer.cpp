#include <crtdbg.h>
#include <iostream>
#include <algorithm>

#include "AEEngine.h"
#include "math.hpp"
#include "renderer.hpp"
#include <string>
#include <iostream>
#include "render_components.hpp"
#include "component.hpp"

namespace {
	AEGfxVertexList* square = 0;
	AEGfxVertexList* triangle = 0;
	AEGfxVertexList* circle = 0;
	s8 pFont{};

	void genSqrMesh() {
		AEGfxMeshStart();
		AEGfxTriAdd(
			-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
			0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
		AEGfxTriAdd(
			0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
			0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 1.0f);
		square = AEGfxMeshEnd();
	}

	void genTriMesh() {
		AEGfxMeshStart();
		AEGfxTriAdd(
			 0.f  , 0.86603f, 0xFFFFFFFF, 1.0f, 1.0f,
			-0.5f , 0		, 0xFFFFFFFF, 1.0f, 1.0f,
			 0.5f ,	0		, 0xFFFFFFFF, 1.0f, 1.0f);
		triangle = AEGfxMeshEnd();
	}

	void genCircMesh() {
		AEGfxMeshStart();

		double baseX = 0.5f;
		double baseY = 0.f;
		for (int i = 0; i < 16; i++) {
			double angle = (2 * PI / 16);
			double rotatedX = cos(angle) * baseX + -sin(angle) * baseY;
			double rotatedY = sin(angle) * baseX + cos(angle) * baseY;
			AEGfxTriAdd(
				0.f, 0.f, 0xFFFFFFFF, 0.f, 0.f,
				baseX, baseY, 0xFFFFFFFF, 1.0f, 1.0f,
				rotatedX, rotatedY, 0xFFFFFFFF, 1.0f, 1.0f);
			baseX = rotatedX;
			baseY = rotatedY;
		}

		// Saving the mesh (list of triangles) in pMesh
		circle = AEGfxMeshEnd();
	}
}



namespace renderSys
{

	void rendererInit() {
		genSqrMesh();
		genTriMesh();
		genCircMesh();
		pFont = AEGfxCreateFont("Assets/liberation-mono.ttf", 72);
		std::cout << "\ninit success\n";
	}

	void DrawRect(RenderData data) {
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
		AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetTransparency(1.0f);

		AEMtx33 transform;
		AEMtx33Identity(&transform);

		// shifts square mesh based on alignment
		if (data.drawmode != MC) {
			AEMtx33 translate;
			switch (data.drawmode) {
				case TL:
					AEMtx33Trans(&translate, 0.5f, -0.5f);
					break;
				case TC:
					AEMtx33Trans(&translate, 0.f, -0.5f);
					break;
				case TR:
					AEMtx33Trans(&translate, -0.5f, -0.5f);
					break;
				case ML:
					AEMtx33Trans(&translate, 0.5f, 0.f);
					break;
				case MC:
					AEMtx33Trans(&translate, 0.f, 0.f);
					break;
				case MR:
					AEMtx33Trans(&translate, -0.5f, 0.f);
					break;
				case BL:
					AEMtx33Trans(&translate, 0.5f, 0.5f);
					break;
				case BC:
					AEMtx33Trans(&translate, 0.f, 0.5f);
					break;
				case BR:
					AEMtx33Trans(&translate, -0.5f, 0.5f);
					break;
			}
			AEMtx33Concat(&transform, &translate, &transform);
		}

		AEMtx33 scale;
		AEMtx33Scale(&scale, data.transform.scale.x, data.transform.scale.y);
		AEMtx33 rotate;
		AEMtx33RotDeg(&rotate, data.transform.rotation);
		AEMtx33 translate;
		AEMtx33Trans(&translate, data.transform.position.x, data.transform.position.y);

		AEMtx33Concat(&transform, &scale, &transform);
		AEMtx33Concat(&transform, &rotate, &transform);
		AEMtx33Concat(&transform, &translate, &transform);
		AEGfxSetTransform(transform.m);

		AEGfxMeshDraw(square, AE_GFX_MDM_TRIANGLES);
	}

	void DrawTri(RenderData data) {
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEMtx33 transform;
		AEMtx33Identity(&transform);
		AEMtx33 scale;
		AEMtx33Scale(&scale, data.transform.scale.x, data.transform.scale.y);
		AEMtx33 rotate;
		AEMtx33RotDeg(&rotate, data.transform.rotation);
		AEMtx33 translate;
		AEMtx33Trans(&translate, data.transform.position.x, data.transform.position.y);

		AEMtx33Concat(&transform, &rotate, &scale);
		AEMtx33Concat(&transform, &translate, &transform);
		AEGfxSetTransform(transform.m);

		AEGfxMeshDraw(triangle, AE_GFX_MDM_TRIANGLES);
	}

	void DrawCirc(RenderData data) {
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEMtx33 transform;
		AEMtx33Identity(&transform);
		AEMtx33 scale;
		AEMtx33Scale(&scale, data.transform.scale.x, data.transform.scale.y);
		AEMtx33 rotate;
		AEMtx33RotDeg(&rotate, data.transform.rotation);
		AEMtx33 translate;
		AEMtx33Trans(&translate, data.transform.position.x, data.transform.position.y);

		AEMtx33Concat(&transform, &rotate, &scale);
		AEMtx33Concat(&transform, &translate, &transform);
		AEGfxSetTransform(transform.m);

		AEGfxMeshDraw(circle, AE_GFX_MDM_TRIANGLES);
	}

	void DrawMyText(char* text, float2 pos, float size) {
		AEGfxPrint(pFont, text, pos.x, pos.y, size, 1.f, 1.f, 1.f, 1.f);
	}
	void DrawArrow(float2 pos)
	{
		//draw rect
		//DrawRect(pos - float2(0,0),0,float2(5,50),center);
		//draw arrow
		//DrawTri(pos + float2(0,25), 0, 25);
	}

	void rendererExit() {
		AEGfxMeshFree(square);
		AEGfxMeshFree(triangle);
		AEGfxMeshFree(circle);
		AEGfxDestroyFont(pFont);
	}
}


//RenderSystem struct functions
void RenderSystem::RegisterRenderer(Renderer* r) 
{ 
	if (!r) return; //check null

	if (_rendererSet.insert(r).second) //insert if not alrdy in set
		_renderers.push_back(r); //only insert to list if ^
}

void RenderSystem::UnregisterRenderer(Renderer* r) 
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

void RenderSystem::FlushRenderers() 
{
	_renderers.clear();
	_rendererSet.clear();
}

void RenderSystem::Draw()
{
	for (auto r : _renderers) r->Draw();
}

RenderSystem& RenderSystem::Instance()
{
	static RenderSystem sys;
	return sys;
}