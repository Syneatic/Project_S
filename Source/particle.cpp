#pragma once

#include "particle.hpp"
#include "physics.hpp"
#include "camera.hpp"

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
					g_pool.collide[i]
					);

			//update pos
			g_pool.pos[i].x += g_pool.vel[i].x * EngineCTX::dt;
			g_pool.pos[i].y += g_pool.vel[i].y * EngineCTX::dt;
		}
		//Debug::Log("Active Particles : ", activeParticles," FPS : ", EngineCTX::frameRate);

	}

	void Render() //by pass our wrapper for performance's sake
	{
		//Debug::ScopedTimer t("p_render");

		for (int i = 0; i < MAX_PARTICLES; ++i) 
		{
			if (!g_pool.active[i]) continue;

			g_pool.color[i].a = 1.0f - (g_pool.time[i] / g_pool.lifetime[i]);

			Graphics::RenderData data{};
			data.alignment = Graphics::Alignment::MC;
			data.blendMode = AE_GFX_BM_BLEND;
			data.color = g_pool.color[i];
			data.drawMode = AE_GFX_MDM_TRIANGLES;
			data.isScreenSpace = false;
			data.layer = g_pool.layer[i];
			data.pos = g_pool.pos[i];
			data.renderMode = AE_GFX_RM_COLOR;
			data.rot = g_pool.rotation[i];
			data.scale = { g_pool.size[i], g_pool.size[i] };
			data.sortOrder = g_pool.sortOrder[i];

			Graphics::Submit(data, Graphics::PrimitiveType::QUAD);
		}
	}

	void Emit(float2 pos, float2 vel, float time, float life,
		Color col, bool shouldCollide,
		FN behaviour, float size, float rotation, Graphics::RenderLayer layer, float sortOrder)
	{
		if (g_pool.freeStackTop < 0) return; // Pool is full

		int index = g_pool.freeStack[g_pool.freeStackTop--];

		g_pool.pos[index]      = pos;
		g_pool.vel[index]      = vel;
		g_pool.time[index]     = time;
		g_pool.lifetime[index] = life;
		g_pool.color[index]    = col;
		g_pool.active[index]   = true;
		g_pool.size[index]     = size;
		g_pool.rotation[index] = rotation;

		g_pool.collide[index]        = shouldCollide;
		g_pool.behaviour[index]      = behaviour;

		g_pool.layer[index] = layer;
		g_pool.sortOrder[index] = sortOrder;
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