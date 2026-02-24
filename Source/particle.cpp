#pragma once

#include "particle.hpp"

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

			//update pos
			g_pool.pos[i].x += g_pool.vel[i].x * dt;
			g_pool.pos[i].y += g_pool.vel[i].y * dt;

			activeParticles++;
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

	void Emit(float2 pos,float2 vel,float life,Color col)
	{
		if (g_pool.freeStackTop < 0) return; // Pool is full

		int index = g_pool.freeStack[g_pool.freeStackTop--];

		g_pool.pos[index] = pos;
		g_pool.vel[index] = vel;
		g_pool.time[index] = 0.0f;
		g_pool.lifetime[index] = life;
		g_pool.color[index] = col;
		g_pool.active[index] = true;
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