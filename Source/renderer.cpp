#include <crtdbg.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <filesystem>
#include <unordered_set>
#include <vector>


#include "AEEngine.h"
#include "math.hpp"
#include "renderer.hpp"
#include "render_components.hpp"
#include "component.hpp"


namespace {
	//NOT EXPOSED!!
	AEGfxVertexList* _quadMesh = 0;
	AEGfxVertexList* _triangleMesh = 0;
	AEGfxVertexList* _circleMesh = 0;
	AEGfxVertexList* _pointMesh = 0;
	s8 pFont{};
	AEGfxTexture* pTex{};
	
	//Renderer List
	std::vector<Renderer*> _renderers;
	std::unordered_set<Renderer*> _rendererSet;

	void GenerateQuadMesh() {
		AEGfxMeshStart();
		AEGfxTriAdd(
			 0.5f, -0.5f, 0xFF'FF'FF'FF, 1.0f, 1.0f,
			-0.5f,  0.5f, 0xFF'FF'FF'FF, 0.0f, 0.0f,
			-0.5f, -0.5f, 0xFF'FF'FF'FF, 0.0f, 1.0f);
		AEGfxTriAdd(
			 0.5f, -0.5f, 0xFF'FF'FF'FF, 1.0f, 1.0f,
			 0.5f,  0.5f, 0xFF'FF'FF'FF, 1.0f, 0.0f,
			-0.5f,  0.5f, 0xFF'FF'FF'FF, 0.0f, 0.0f);
		_quadMesh = AEGfxMeshEnd();
	}

	void GenerateTriMesh() {
		AEGfxMeshStart();
		AEGfxTriAdd(
			 0.f  , 0.86603f, 0xFFFFFFFF, 1.0f, 1.0f,
			-0.5f , 0		, 0xFFFFFFFF, 1.0f, 1.0f,
			 0.5f ,	0		, 0xFFFFFFFF, 1.0f, 1.0f);
		_triangleMesh = AEGfxMeshEnd();
	}

	void GenerateCircleMesh() {
		AEGfxMeshStart();

		//definitely needs to be change to f32 for no conversion
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
		_circleMesh = AEGfxMeshEnd();
	}

	void GeneratePointMesh()
	{
		AEGfxMeshStart();
		AEGfxVertexAdd(0.f,0.f,0xFFFFFFFF,0.f,0.f);
		_pointMesh = AEGfxMeshEnd();
	}
}



namespace RenderSystem
{
	//call before entering game loop
	void RendererInitialize() {
		GenerateQuadMesh();
		GenerateTriMesh();
		GenerateCircleMesh();
		pFont = AEGfxCreateFont("./Assets/liberation-mono.ttf", 72);
		pTex = AEGfxTextureLoad("./Assets/PlanetTexture.png");
		std::cout << "\ninit success\n";
	}

	//exposed api for drawing renderers
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
		for (auto r : _renderers) r->Draw();
	}

	void SetTransform(Transform t, Alignment mode, AEMtx33& mtx)
	{
		AEMtx33Identity(&mtx);

		AEMtx33 align{};
		switch (mode) {
		case TL:
			AEMtx33Trans(&align, 0.5f, -0.5f);
			break;
		case TC:
			AEMtx33Trans(&align, 0.f, -0.5f);
			break;
		case TR:
			AEMtx33Trans(&align, -0.5f, -0.5f);
			break;
		case ML:
			AEMtx33Trans(&align, 0.5f, 0.f);
			break;
		case MC:
			AEMtx33Trans(&align, 0.f, 0.f);
			break;
		case MR:
			AEMtx33Trans(&align, -0.5f, 0.f);
			break;
		case BL:
			AEMtx33Trans(&align, 0.5f, 0.5f);
			break;
		case BC:
			AEMtx33Trans(&align, 0.f, 0.5f);
			break;
		case BR:
			AEMtx33Trans(&align, -0.5f, 0.5f);
			break;
		}
		AEMtx33Concat(&mtx, &align, &mtx);

		AEMtx33 scale{};
		AEMtx33Scale(&scale, t.scale.x, t.scale.y);
		AEMtx33 rotate{};
		AEMtx33RotDeg(&rotate, t.rotation);
		AEMtx33 translate{}; //reset
		AEMtx33Trans(&translate, t.position.x, t.position.y);

		AEMtx33Concat(&mtx, &scale, &mtx);
		AEMtx33Concat(&mtx, &rotate, &mtx);
		AEMtx33Concat(&mtx, &translate, &mtx);
	}

	//draw primitives/text
	void DrawPoint(float2 pos, Color objColor)
	{
		//set modes
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);

		//set colors
		AEGfxSetColorToAdd(0, 0, 0, 0);
		AEGfxSetBlendColor(0, 0, 0, 0);
		AEGfxSetColorToMultiply(objColor.r, objColor.g, objColor.b, 1);
		AEGfxSetTransparency(1.f);

		AEMtx33 transform;
		AEMtx33Identity(&transform);
		AEMtx33 scale;
		AEMtx33Scale(&scale, 5.f, 5.f);
		AEMtx33 translate;
		AEMtx33Trans(&translate, pos.x, pos.y);
		AEMtx33Concat(&transform, &transform, &scale);
		AEMtx33Concat(&transform, &translate, &transform);
		AEGfxSetTransform(transform.m);

		AEGfxMeshDraw(_quadMesh, AE_GFX_MDM_TRIANGLES);
	}

	void DrawQuad(RenderData data) 
	{
		//set modes
		AEGfxSetRenderMode(data.renderMode);
		AEGfxSetBlendMode(data.blendMode);
	
		//set colors
		AEGfxSetColorToAdd(0, 0, 0, 0);
		AEGfxSetBlendColor(0, 0, 0, 0);
		AEGfxSetColorToMultiply(data.color.r, data.color.g, data.color.b, 1);
		AEGfxSetTransparency(data.color.a);

		//set texture only if using texture mode
		if (data.renderMode == AE_GFX_RM_TEXTURE)
			AEGfxTextureSet(pTex, 0.f, 0.f);
		else
			AEGfxTextureSet(nullptr, 0.f, 0.f);

		AEMtx33 transform{};
		SetTransform(data.transform,data.alignment,transform);
		AEGfxSetTransform(transform.m);

		AEGfxMeshDraw(_quadMesh, data.meshMode);
	}

	void DrawTri(RenderData data) {
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToMultiply(data.color.r, data.color.g, data.color.b, data.color.a);

		AEMtx33 transform;
		SetTransform(data.transform, data.alignment, transform);
		AEGfxSetTransform(transform.m);

		AEGfxMeshDraw(_triangleMesh, AE_GFX_MDM_TRIANGLES);
	}

	void DrawCircle(RenderData data) 
	{
		AEGfxSetRenderMode(data.renderMode);
		AEGfxSetBlendMode(data.blendMode);

		AEGfxSetColorToAdd(0, 0, 0, 0);
		AEGfxSetBlendColor(0, 0, 0, 0);
		AEGfxSetColorToMultiply(data.color.r, data.color.g, data.color.b, 1.0f);
		AEGfxSetTransparency(data.color.a);

		AEGfxTextureSet(nullptr, 0.f, 0.f);

		AEMtx33 transform{};
		SetTransform(data.transform, data.alignment, transform);
		AEGfxSetTransform(transform.m);

		AEGfxMeshDraw(_circleMesh, data.meshMode);
	}

	void DrawMyText(const char* text, RenderData data) {
		f32 screenPosX = data.transform.position.x / AEGfxGetWindowWidth();
		f32 screenPosY = data.transform.position.y / AEGfxGetWindowHeight();
		f32 textWidth{}, textHeight{}, offX{}, offY{};
		AEGfxGetPrintSize(pFont, text, data.transform.scale.x, &textWidth, &textHeight);
		switch (data.alignment) {
		case TL:
			offX = 0; offY = textHeight;
			break;
		case TC:
			offX = textWidth / 2; offY = textHeight;
			break;
		case TR:
			offX = textWidth; offY = textHeight;
			break;
		case ML:
			offX = 0; offY = textHeight / 2;
			break;
		case MC:
			offX = textWidth / 2; offY = textHeight / 2;
			break;
		case MR:
			offX = textWidth; offY = textHeight / 2;
			break;
		case BL:
			offX = 0; offY = 0;
			break;
		case BC:
			offX = textWidth / 2; offY = 0;
			break;
		case BR:
			offX = textWidth; offY = 0;
			break;
		}
		AEGfxPrint(pFont, text, screenPosX -offX, screenPosY-offY, data.transform.scale.x, data.color.r, data.color.g, data.color.b, data.color.a);
	}

	void DrawArrow(RenderData data)
	{
		// 1. Draw the Stem (Quad)
		RenderData stem = data;
		stem.transform.scale = { data.transform.scale.x, data.transform.scale.y * 0.2f }; // Thin stem
		stem.alignment = ML; // Align to start at position
		DrawQuad(stem);

		// 2. Draw the Tip (Triangle)
		RenderData tip = data;
		// Offset tip to the end of the stem
		float angleRad = data.transform.rotation * (PI / 180.0f);
		tip.transform.position.x += cosf(angleRad) * data.transform.scale.x;
		tip.transform.position.y += sinf(angleRad) * data.transform.scale.x;

		tip.transform.scale = { data.transform.scale.y, data.transform.scale.y };
		tip.transform.rotation -= 90.0f; // Adjust tri mesh orientation to point along axis
		tip.alignment = MC;
		DrawTri(tip);
	}

	//call after game loop
	void RendererExit() {
		AEGfxMeshFree(_quadMesh);
		AEGfxMeshFree(_triangleMesh);
		AEGfxMeshFree(_circleMesh);
		//AEGfxMeshFree(_pointMesh);

		AEGfxDestroyFont(pFont);
		AEGfxTextureUnload(pTex);
	}
}


