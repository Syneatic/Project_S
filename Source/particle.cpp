#pragma once

#include "particle.hpp"
#include "physics.hpp"
#include "ImGUI/imgui.h"

namespace
{
	void RecursiveEmit(Physics::RaycastHit hit,float2 vel,float lifetime,int childrenToSpawn,Color color)
	{
		float2 reflectedVel = reflect(vel, hit.normal);
		float speed = length(vel);

		if (childrenToSpawn > 4)
		{
			int nextBurstCount = childrenToSpawn / 4;

			float baseAngle = atan2f(hit.normal.y, hit.normal.x); // Angle of the wall normal
			float splashRange = PI;

			Color startCol = color;
			Color endCol = { 1.0f, 1.0f, 0.0f, 1.0f }; // Example: Shift towards a deep blue


			float t = 1.0f - (float)nextBurstCount / 128.f;

			Color nextColor;
			nextColor.r = startCol.r + (endCol.r - startCol.r) * t; // Shift 40% per bounce
			nextColor.g = startCol.g + (endCol.g - startCol.g) * t;
			nextColor.b = startCol.b + (endCol.b - startCol.b) * t;
			nextColor.a = startCol.a;

			for (int j = 0; j < childrenToSpawn; ++j)
			{
				//constraint to 180
				float offset = (j / (float)nextBurstCount) * splashRange - (splashRange / 2.0f);
				float finalAngle = baseAngle + offset;

				float2 spawnDir = { cosf(finalAngle), sinf(finalAngle) };

				ParticleSystem::Emit(
					hit.point + (hit.normal * 1.0f),
					spawnDir * speed,
					lifetime * 0.8f,
					nextColor,
					true,            // These children will also collide and echo
					nextBurstCount,   // Pass the reduced count for the next generation
					nullptr
				);
			}
		}
		
	}
}


//SoA approach
namespace ParticleSystem
{
	Pool g_pool;

	void Initialize()
	{
		g_pool.freeStackTop = MAX_PARTICLES - 1;
		//set all to inactive
		for (int i = 0; i < MAX_PARTICLES; ++i) {
			g_pool.freeStack[i] = i; //reset stack
			g_pool.active[i] = false;
		}
	}

	void Update()
	{
		f64 dt = AEFrameRateControllerGetFrameTime();
		int activeParticles{};
		for (int i = 0; i < MAX_PARTICLES; ++i) {
			if (!g_pool.active[i]) continue; //skip inactive

			g_pool.time[i] += dt; //inc time

			if (g_pool.time[i] >= g_pool.lifetime[i]) //cull dead particles
			{
				g_pool.active[i] = false;
				g_pool.freeStack[++g_pool.freeStackTop] = i;
				continue;
			}

			activeParticles++;

			//update pos
			g_pool.pos[i].x += g_pool.vel[i].x * dt;
			g_pool.pos[i].y += g_pool.vel[i].y * dt;

			//execute behaviour
			if (g_pool.behaviour[i]) 
				g_pool.behaviour[i](
					g_pool.pos[i], 
					g_pool.vel[i], 
					g_pool.lifetime[i], 
					g_pool.color[i], 
					g_pool.collide[i], 
					g_pool.burstRemaining[i]
					);
		}

		std::cout << "Active Particles : " << activeParticles << ", FPS : " << AEFrameRateControllerGetFrameRate() << "\n";
	}

	void Render() //by pass our wrapper for performance's sake
	{
		//batch this render states
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);

		AEGfxSetColorToAdd(0, 0, 0, 0);
		AEGfxSetBlendColor(0, 0, 0, 0);
		AEGfxSetTransparency(1.f);

		AEMtx33 scaleMtx;
		AEMtx33Scale(&scaleMtx, 5.0f, 5.0f);

		for (int i = 0; i < MAX_PARTICLES; ++i) 
		{
			if (!g_pool.active[i]) continue;

			//lower alpha of dying particles
			float alpha = 1.0f - (g_pool.time[i] / g_pool.lifetime[i]);
			AEGfxSetColorToMultiply(g_pool.color[i].r, g_pool.color[i].g, g_pool.color[i].b, alpha);

			//construct matrix
			AEMtx33 transMtx, finalMtx;
			AEMtx33Trans(&transMtx, g_pool.pos[i].x, g_pool.pos[i].y);
			AEMtx33Concat(&finalMtx, &transMtx, &scaleMtx);

			AEGfxSetTransform(finalMtx.m);
			
			AEGfxMeshDraw(RenderSystem::GetQuadMesh(), AE_GFX_MDM_TRIANGLES);
		}
	}

	void Emit(float2 pos,float2 vel,float life,Color col, bool shouldCollide, int burstLimit, FN behaviour)
	{
		if (g_pool.freeStackTop < 0) return; // Pool is full

		int index = g_pool.freeStack[g_pool.freeStackTop--];

		g_pool.pos[index] = pos;
		g_pool.vel[index] = vel;
		g_pool.time[index] = 0.0f;
		g_pool.lifetime[index] = life;
		g_pool.color[index] = col;
		g_pool.active[index] = true;

		g_pool.burstRemaining[index] = burstLimit;
		g_pool.collide[index] = shouldCollide;

		g_pool.behaviour[index] = behaviour;
	}

	void Flush()
	{
		g_pool.freeStackTop = MAX_PARTICLES - 1;
		//set all to inactive
		for (int i = 0; i < MAX_PARTICLES; ++i) {
			g_pool.freeStack[i] = i; //reset stack
			g_pool.active[i] = false;
		}
	}
}