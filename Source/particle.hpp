#pragma once

#include "math.hpp"
#include "renderer.hpp"

//SoA approach
namespace ParticleSystem
{
	using FN = void(*)(float2& pos, float2& vel,float& time, float& lifetime, Color& col, bool& shouldCollide, int& burstLimit);

	const int MAX_PARTICLES = 5096 * 8;

	struct Pool
	{
		float2 pos[MAX_PARTICLES]{}; //might change to separated x,y
		float size[MAX_PARTICLES]{};
		float rotation[MAX_PARTICLES]{};

		float2 vel[MAX_PARTICLES]{}; //same here

		bool timeScale[MAX_PARTICLES]{};
		float  time[MAX_PARTICLES]{};
		float  lifetime[MAX_PARTICLES]{};

		Color  color[MAX_PARTICLES]{}; //maybe this too
		bool   active[MAX_PARTICLES]{};

		int burstRemaining[MAX_PARTICLES]{}; //keep track of how many generation of reflection remain
		bool collide[MAX_PARTICLES]{};
		FN behaviour[MAX_PARTICLES]{nullptr};

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
	void Emit(float2 pos, float2 vel, float time, float life,
		Color col, bool shouldCollide, int burstLimit,
		FN behaviour, float size = 5.0f, float rotation = 0.0f,
		bool timeScale = true);
	void Flush();
}