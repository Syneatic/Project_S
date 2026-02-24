#pragma once

#include "math.hpp"
#include "renderer.hpp"

//SoA approach
namespace ParticleSystem
{
	const int MAX_PARTICLES = 5096 * 8;

	struct Pool
	{
		float2 pos[MAX_PARTICLES]{};
		float2 vel[MAX_PARTICLES]{};
		float  time[MAX_PARTICLES]{};
		float  lifetime[MAX_PARTICLES]{};
		Color  color[MAX_PARTICLES]{};
		bool   active[MAX_PARTICLES]{};

		int burstRemaining[MAX_PARTICLES]{}; //keep track of how many generation of reflection remain
		bool collide[MAX_PARTICLES]{};

		int freeStack[MAX_PARTICLES]{};
		int freeStackTop = -1;
	};

	struct Emitter
	{
		float2 pos;
		float2 dir; //direction to launch
		float spread;
		float spd;
		float spawnRate;
		float accumulator;
		Color color;
		float lifetime;
	};

	void Initialize();
	void Update();
	void Render();
	void Emit(float2 pos, float2 vel, float life, Color col, bool shouldCollide, int burstLimit);
	void Flush();
}