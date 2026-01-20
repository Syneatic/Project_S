#include <crtdbg.h>
#include "AEEngine.h"
#include "math.hpp"
#include "renderer.hpp"
#include "component.hpp"
#include <iostream>

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
			-1.f, -1.f, 0xFFFFFFFF, 0.0f, 1.0f,
			1.f, 0.f, 0xFFFFFFFF, 1.0f, 1.0f,
			0.f, 1.f, 0xFFFFFFFF, 0.0f, 0.0f);
		AEGfxTriAdd(
			1.f, 1.f, 0xFFFFFFFF, 1.0f, 1.0f,
			0.f, 1.f, 0xFFFFFFFF, 1.0f, 0.0f,
			1.f, 0.f, 0xFFFFFFFF, 0.0f, 0.0f);
		centerSqr = AEGfxMeshEnd();
	}
	void genTriMesh() {

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

void renderSys::drawRect(Transform objInfo, MeshRenderer renderInfo, drawMode alignment) {
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEMtx33 transform;
	AEMtx33Identity(&transform);
	AEMtx33 scale;
	AEMtx33Scale(&scale, objInfo.scale.x, objInfo.scale.y);
	AEMtx33 rotate;
	AEMtx33RotDeg(&rotate, objInfo.rotation);
	AEMtx33 translate;
	AEMtx33Trans(&translate, objInfo.position.x, objInfo.position.y);

	AEMtx33Concat(&transform, &rotate, &scale);
	AEMtx33Concat(&transform, &translate, &transform);
	AEGfxSetTransform(transform.m);

	if (alignment == lSide) {
		AEGfxMeshDraw(lSideSqr, AE_GFX_MDM_TRIANGLES);
	}
	else {
		AEGfxMeshDraw(centerSqr, AE_GFX_MDM_TRIANGLES);
	}
	std::cout << "Draw ok" << "\n";
}

void renderSys::drawTri(Transform objInfo, MeshRenderer renderInfo) {
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
}

void renderSys::drawCirc(Transform objInfo, MeshRenderer renderInfo) {
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEMtx33 transform;
	AEMtx33Identity(&transform);
	AEMtx33 scale;
	AEMtx33Scale(&scale, objInfo.scale.x, objInfo.scale.y);
	AEMtx33 rotate;
	AEMtx33RotDeg(&rotate, objInfo.rotation);
	AEMtx33 translate;
	AEMtx33Trans(&translate, objInfo.position.x, objInfo.position.y);

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
