#include <crtdbg.h>
#include <iostream>
#include <algorithm>

#include "AEEngine.h"
#include "math.hpp"
#include "renderer.hpp"
#include "render_components.hpp"

namespace {
	AEGfxVertexList* lSideSqr = 0;
	AEGfxVertexList* centerSqr = 0;
	AEGfxVertexList* triangle = 0;
	AEGfxVertexList* circle = 0;

	void genSqrMesh() {
		AEGfxMeshStart();
		AEGfxTriAdd(
			0.f, 0.f, 0xFFFFFFFF, 1.0f, 1.0f,
			1.f, 0.f, 0xFFFFFFFF, 1.0f, 1.0f,
			1.f, 1.f, 0xFFFFFFFF, 1.0f, 1.0f);
		AEGfxTriAdd(
			1.f, 1.f, 0xFFFFFFFF, 1.0f, 1.0f,
			0.f, 1.f, 0xFFFFFFFF, 1.0f, 1.0f,
			0.f, 0.f, 0xFFFFFFFF, 1.0f, 1.0f);
		lSideSqr = AEGfxMeshEnd();

		AEGfxMeshStart();
		AEGfxTriAdd(
			-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
			0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
		AEGfxTriAdd(
			0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
			0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
		centerSqr = AEGfxMeshEnd();
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

void renderSys::rendererInit() {
	genSqrMesh();
	genTriMesh();
	genCircMesh();
	std::cout << "\ninit success\n";
}

void renderSys::drawRect(float2 pos, float rotAngle, float2 size, drawMode alignment) {
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetTransparency(1.0f);

	AEMtx33 transform;
	AEMtx33Identity(&transform);
	AEMtx33 scale;
	AEMtx33Scale(&scale, size.x, size.y);
	AEMtx33 rotate;
	AEMtx33RotDeg(&rotate, rotAngle);
	AEMtx33 translate;
	AEMtx33Trans(&translate, pos.x, pos.y);

	AEMtx33Concat(&transform, &rotate, &scale);
	AEMtx33Concat(&transform, &translate, &transform);
	AEGfxSetTransform(transform.m);

	if (alignment == lSide) {
		AEGfxMeshDraw(lSideSqr, AE_GFX_MDM_TRIANGLES);
	}
	else {
		AEGfxMeshDraw(centerSqr, AE_GFX_MDM_TRIANGLES);
	}
}

void renderSys::drawTri(float2 pos, float rotAngle, float size) {
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEMtx33 transform;
	AEMtx33Identity(&transform);
	AEMtx33 scale;
	AEMtx33Scale(&scale, size, size);
	AEMtx33 rotate;
	AEMtx33RotDeg(&rotate, rotAngle);
	AEMtx33 translate;
	AEMtx33Trans(&translate, pos.x, pos.y);

	AEMtx33Concat(&transform, &rotate, &scale);
	AEMtx33Concat(&transform, &translate, &transform);
	AEGfxSetTransform(transform.m);

	AEGfxMeshDraw(triangle, AE_GFX_MDM_TRIANGLES);
}

void renderSys::drawCirc(float2 pos, float rotAngle, float size) {
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEMtx33 transform;
	AEMtx33Identity(&transform);
	AEMtx33 scale;
	AEMtx33Scale(&scale, size, size);
	AEMtx33 rotate;
	AEMtx33RotDeg(&rotate, rotAngle);
	AEMtx33 translate;
	AEMtx33Trans(&translate, pos.x, pos.y);

	AEMtx33Concat(&transform, &rotate, &scale);
	AEMtx33Concat(&transform, &translate, &transform);
	AEGfxSetTransform(transform.m);

	AEGfxMeshDraw(circle, AE_GFX_MDM_TRIANGLES);
}



void renderSys::rendererExit() {
	AEGfxMeshFree(lSideSqr);
	AEGfxMeshFree(centerSqr);
	AEGfxMeshFree(triangle);
	AEGfxMeshFree(circle);
}

namespace renderSys
{
	void DrawArrow(float2 pos)
	{
		//draw rect
		drawRect(pos - float2(0,0),0,float2(5,50),center);
		//draw arrow
		drawTri(pos + float2(0,25), 0, 25);
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