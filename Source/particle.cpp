#pragma once

#include "particle.hpp"
#include "physics.hpp"

namespace
{
		
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
		//Debug::ScopedTimer t("p_update");
		int activeParticles{};
		
		for (int i = 0; i < MAX_PARTICLES; ++i) 
		{
			if (!g_pool.active[i]) continue; //skip inactive
			g_pool.time[i] += EngineCTX::dt; //inc time

			if (g_pool.time[i] >= g_pool.lifetime[i]) //cull dead particles
			{
				g_pool.active[i] = false;
				g_pool.freeStack[++g_pool.freeStackTop] = i;
				continue;
			}

			activeParticles++;

			if (g_pool.vel[i] == float2::zero()) continue; //skip stationary

			//execute behaviour
			if (g_pool.behaviour[i]) 
				g_pool.behaviour[i](
					g_pool.pos[i], 
					g_pool.vel[i],
					g_pool.time[i],
					g_pool.lifetime[i], 
					g_pool.color[i], 
					g_pool.collide[i], 
					g_pool.burstRemaining[i]
					);

			//update pos
			g_pool.pos[i].x += g_pool.vel[i].x * (g_pool.timeScale[i] ? EngineCTX::dt : EngineCTX::unscaledDt);
			g_pool.pos[i].y += g_pool.vel[i].y * (g_pool.timeScale[i] ? EngineCTX::dt : EngineCTX::unscaledDt);
		}
		//Debug::Log("Active Particles : ", activeParticles," FPS : ", EngineCTX::frameRate);

	}

	void Render() //by pass our wrapper for performance's sake
	{
		//Debug::ScopedTimer t("p_render");
		
		//batch this render states
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);

		AEGfxSetColorToAdd(0, 0, 0, 0);
		AEGfxSetBlendColor(0, 0, 0, 0);
		AEGfxSetTransparency(1.f);

		for (int i = 0; i < MAX_PARTICLES; ++i) 
		{
			if (!g_pool.active[i]) continue;

			//lower alpha of dying particles
			float alpha = 1.0f - (g_pool.time[i] / g_pool.lifetime[i]);
			AEGfxSetColorToMultiply(g_pool.color[i].r, g_pool.color[i].g, g_pool.color[i].b, alpha);

			//construct matrix
			AEMtx33 scaleMtx, rotMtx, transMtx, finalMtx, temp;
			AEMtx33Scale(&scaleMtx, g_pool.size[i], g_pool.size[i]);
			AEMtx33Rot(&rotMtx, g_pool.rotation[i]);
			AEMtx33Trans(&transMtx, g_pool.pos[i].x, g_pool.pos[i].y);

			AEMtx33Concat(&temp, &rotMtx, &scaleMtx);
			AEMtx33Concat(&finalMtx, &transMtx, &temp);

			AEGfxSetTransform(finalMtx.m);
			
			AEGfxMeshDraw(Graphics::QuadMesh(), AE_GFX_MDM_TRIANGLES);
		}
	}

	void Emit(float2 pos, float2 vel, float time, float life,
		Color col, bool shouldCollide, int burstLimit,
		FN behaviour, float size, float rotation, bool timeScale)
	{
		if (g_pool.freeStackTop < 0) return; // Pool is full

		int index = g_pool.freeStack[g_pool.freeStackTop--];

		g_pool.pos[index]      = pos;
		g_pool.vel[index]      = vel;
		g_pool.timeScale[index]= timeScale;
		g_pool.time[index]     = time;
		g_pool.lifetime[index] = life;
		g_pool.color[index]    = col;
		g_pool.active[index]   = true;
		g_pool.size[index]     = size;
		g_pool.rotation[index] = rotation;

		g_pool.burstRemaining[index] = burstLimit;
		g_pool.collide[index]        = shouldCollide;
		g_pool.behaviour[index]      = behaviour;
	}

	// Overload to pass a builder object by reference to create a pool.
	void Emit(PoolBuilder const& pb)
	{
		if (g_pool.freeStackTop < 0) return; // Pool is full

		int index = g_pool.freeStack[g_pool.freeStackTop--];

		g_pool.pos[index] = pb.pos;
		g_pool.vel[index] = pb.vel;
		g_pool.timeScale[index] = pb.timescale;
		g_pool.time[index] = pb.time;
		g_pool.lifetime[index] = pb.life;
		g_pool.color[index] = pb.col;
		g_pool.active[index] = true;
		g_pool.size[index] = pb.size;
		g_pool.rotation[index] = pb.rotation;

		g_pool.burstRemaining[index] = pb.burstLimit;
		g_pool.collide[index] = pb.shouldCollide;
		g_pool.behaviour[index] = pb.behaviour;
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