#pragma once
#include <ostream>
#include "AETypes.h"
constexpr float kEps = 1e-6f;

struct float2
{
	f32 x{};
	f32 y{};

	float2() : x(0), y(0) {};
	float2(f32 ix, f32 iy) : x(ix), y(iy) {};

	static float2 zero() { return float2{ 0.f,0.f }; }

	//operator overload
	float2 operator+ (const float2 rhs) const;

	float2 operator-(const float2& rhs) const;

	float2& operator+=(const float2& rhs);

	float2& operator-=(const float2& rhs);

	float2 operator* (const float rhs) const;

	static float2 up()		{ return float2(0.f , 1.f); }
	static float2 down()	{ return float2(0.f ,-1.f); }
	static float2 left()	{ return float2(-1.f, 0.f); }
	static float2 right()	{ return float2(1.f , 0.f); }

};

std::ostream& operator<< (std::ostream& o, const float2& f);

float absf(f32 a);
float dot(float2 a, float2 b);
float lengthsq(float2 a);
float length(float2 a);

float2 normalize(const float2& a);
float2 reflect(const float2& v, const float2& n);
float2 lerp(const float2& a, const float2& b, float t);

namespace Random
{
	float RandFloat(float min, float max);
	int RandInt(int min, int max);
}

