#pragma once

#include "math.hpp"
#include "renderer.hpp"

struct Particle
{
	float2 pos{};
	float2 vel{};
	float time{};
	float lifetime{};
	bool active = true;
	bool stay = false;
};

struct ParticleEmitters
{
	int* indices = nullptr; //8
	int aliveCount = 0; //4
	int maxLocal = 0; //4
};

namespace
{
	const int MAX_PARTICLES = 5096;
	//pool
	Particle _particlePool[MAX_PARTICLES]{};
	int		 _freeStack[MAX_PARTICLES]{};
	int		 _freeStackTop = -1;

}


namespace ParticleSystem
{
	//pool

}