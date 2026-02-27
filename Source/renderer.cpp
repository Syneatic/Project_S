#include "math.hpp"
#include "renderer.hpp"
#include "camera.hpp"
#include "render_components.hpp"
#include "particle.hpp"

//fwd decl
void AddVertex(float2 p, Color c, float2 uv)
{
	AEGfxVertexAdd(p.x, p.y, c.hex(), uv.x, uv.y);
}
namespace {
	//NOT EXPOSED!!
	AEGfxVertexList* _quadMesh = 0;
	AEGfxVertexList* _triangleMesh = 0;
	AEGfxVertexList* _circleMesh = 0;
	AEGfxVertexList* _pointMesh = 0;

	//only for gizmos
	AEGfxVertexList* _boxMesh = 0;

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

		//AEGfxTriAdd(
		//	0.0f, 0.57735f, 0xFFFFFFFF, 1.0f, 1.0f,
		//	-0.5f, -0.28867f, 0xFFFFFFFF, 1.0f, 1.0f,
		//	0.5f, -0.28867f, 0xFFFFFFFF, 1.0f, 1.0f);
		_triangleMesh = AEGfxMeshEnd();
	}

	void GenerateCircleMesh() {
		AEGfxMeshStart();

		//definitely needs to be change to f32 for no conversion
		/*f32 baseX = 0.5f;
		f32 baseY = 0.f;
		for (int i = 0; i < 16; i++) {
			f32 angle = (2.f * PI / 16.f);
			f32 rotatedX = cos(angle) * baseX + -sin(angle) * baseY;
			f32 rotatedY = sin(angle) * baseX + cos(angle) * baseY;
			AEGfxTriAdd(
				0.f, 0.f, 0xFFFFFFFF, 0.f, 0.f,
				baseX, baseY, 0xFFFFFFFF, 1.0f, 1.0f,
				rotatedX, rotatedY, 0xFFFFFFFF, 1.0f, 1.0f);
			baseX = rotatedX;
			baseY = rotatedY;
		}*/


		const int segments = 32; //resolution of circle
		const f32 radius = 0.5f; //produces a unit circle (dia = 1)
		const f32 angleStep = (2.0f * PI) / segments;

		for (int i = 0; i < segments; i++)
		{
			f32 theta1 = i * angleStep;
			f32 theta2 = (i + 1) * angleStep;

			//vertices
			float2 v0 = { 0.0f, 0.0f }; // center
			float2 v1 = { cosf(theta1) * radius, sinf(theta1) * radius };
			float2 v2 = { cosf(theta2) * radius, sinf(theta2) * radius };

			float2 uv0 = { 0.5f, 0.5f };
			float2 uv1 = { (v1.x + 0.5f), (v1.y + 0.5f) };
			float2 uv2 = { (v2.x + 0.5f), (v2.y + 0.5f) };

			AEGfxTriAdd(
				v0.x, v0.y, 0xFFFFFFFF, 0.f, 0.f,
				v1.x, v1.y, 0xFFFFFFFF, 1.0f, 1.0f,
				v2.x, v2.y, 0xFFFFFFFF, 1.0f, 1.0f);
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

	void GenerateBoxMesh()
	{
		Color c(1.f, 1.f, 1.f);
		AEGfxMeshStart();

		//top line
		AddVertex(float2(-0.5, 0.5), c, float2(0, 0));
		AddVertex(float2(0.5, 0.5), c,  float2(0, 0));

		//left
		AddVertex(float2(-0.5, 0.5), c, float2(0, 0));
		AddVertex(float2(-0.5, -0.5), c, float2(0, 0));

		//right
		AddVertex(float2(0.5, 0.5), c, float2(0, 0));
		AddVertex(float2(0.5, -0.5), c, float2(0, 0));

		//bottom
		AddVertex(float2(-0.5, -0.5), c, float2(0, 0));
		AddVertex(float2(0.5, -0.5), c, float2(0, 0));


		_boxMesh = AEGfxMeshEnd();
	}
}



namespace RenderSystem
{
	//call before entering game loop
	void RendererInitialize() {
		GenerateQuadMesh();
		GenerateTriMesh();
		GenerateCircleMesh();
		GenerateBoxMesh();

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
		for (auto r : _renderers) {
			if (r->renderLayer < UI) r->Draw();
		}

		ParticleSystem::Render();

		for (auto r : _renderers) {
			if (r->renderLayer>= UI) r->Draw();
		}
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

	void SetTransform(float2 t, float2 s, f32 r, Alignment mode, AEMtx33& mtx)
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
		AEMtx33Scale(&scale, s.x, s.y);
		AEMtx33 rotate{};
		AEMtx33RotDeg(&rotate, r);
		AEMtx33 translate{};
		AEMtx33Trans(&translate, t.x, t.y);

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
		AEMtx33Concat(&transform, &CameraData::camMatrix, &transform);
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
		AEMtx33Concat(&transform, &CameraData::camMatrix, &transform);
		AEGfxSetTransform(transform.m);

		AEGfxMeshDraw(_quadMesh, data.meshMode);
	}

	void DrawTri(RenderData data) {
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToMultiply(data.color.r, data.color.g, data.color.b, data.color.a);

		AEMtx33 transform;
		SetTransform(data.transform, data.alignment, transform);
		AEMtx33Concat(&transform, &CameraData::camMatrix, &transform);
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
		AEMtx33Concat(&transform, &CameraData::camMatrix, &transform);
		AEGfxSetTransform(transform.m);

		AEGfxMeshDraw(_circleMesh, data.meshMode);
	}

	void DrawMyText(const char* text, RenderData data) 
	{
		f32 winW = (f32)AEGfxGetWindowWidth();
		f32 winH = (f32)AEGfxGetWindowHeight();
		f32 halfW = winW * 0.5f;
		f32 halfH = winH * 0.5f;

		float2 screenPixelPos = data.transform.position;
		f32 aeX = screenPixelPos.x / halfW;
		f32 aeY = screenPixelPos.y / halfH;

		f32 textW, textH;
		AEGfxGetPrintSize(pFont, text, data.transform.scale.x, &textW, &textH);

		f32 offX = 0, offY = 0;
		switch (data.alignment)
		{
		case Alignment::TL:
			offX = 0;           offY = textH;           break;
		case Alignment::TC:
			offX = textW * 0.5f; offY = textH;           break;
		case Alignment::TR:
			offX = textW;        offY = textH;           break;

		case Alignment::ML:
			offX = 0;           offY = textH * 0.5f; break;
		case Alignment::MC:
			offX = textW * 0.5f; offY = textH * 0.5f; break;
		case Alignment::MR:
			offX = textW;        offY = textH * 0.5f; break;

		case Alignment::BL:
			offX = 0;           offY = 0;        break;
		case Alignment::BC:
			offX = textW * 0.5f; offY = 0;        break;
		case Alignment::BR:
			offX = textW;        offY = 0;        break;
		}

		AEGfxPrint(pFont, text, aeX - offX, aeY - offY,
			data.transform.scale.x, data.color.r, data.color.g, data.color.b, data.color.a);
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

	//not used for gameobject rendering
	void DrawBox(float2 p,float2 scl,f32 rot,Color c)
	{
		//std::cout << scl.x << ", " << scl.y << std::endl;

		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetBlendMode(AE_GFX_BM_NONE);
		AEGfxSetColorToMultiply(c.r, c.g, c.b, 1.f);
		AEGfxSetTransparency(1.f);
		AEMtx33 transform{};
		SetTransform(p, scl, rot, Alignment::MC, transform);
		AEMtx33Concat(&transform, &CameraData::camMatrix, &transform);
		AEGfxSetTransform(transform.m);

		AEGfxMeshDraw(_boxMesh, AE_GFX_MDM_LINES);
	}


	AEGfxVertexList* GetQuadMesh() { return _quadMesh; }


	//call after game loop
	void RendererExit() 
	{
		AEGfxMeshFree(_quadMesh);
		AEGfxMeshFree(_triangleMesh);
		AEGfxMeshFree(_circleMesh);
		AEGfxMeshFree(_boxMesh);
		//AEGfxMeshFree(_pointMesh);

		AEGfxDestroyFont(pFont);
		AEGfxTextureUnload(pTex);
	}
}


