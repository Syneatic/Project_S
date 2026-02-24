#pragma once
#include <iostream>
#include <cmath>

#include "AETypes.h"
namespace
{
	constexpr float kEps = 1e-6f;
}

struct float2
{
	f32 x{};
	f32 y{};

	float2() : x(0), y(0) {};
	float2(f32 ix, f32 iy) : x(ix), y(iy) {};

	static float2 zero() { return float2{ 0.f,0.f }; }

	//operator overload
	float2 operator+ (const float2 rhs) const
	{
		return float2(x + rhs.x, y + rhs.y);
	}

	float2 operator-(const float2& rhs) const
	{
		return float2(x - rhs.x, y - rhs.y);
	}

	float2& operator+=(const float2& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	}

	float2& operator-=(const float2& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	}

	float2 operator* (const float rhs) const
	{
		return float2(x * rhs, y * rhs);
	}

	static float2 up()		{ return float2(0.f , 1.f); }
	static float2 down()	{ return float2(0.f ,-1.f); }
	static float2 left()	{ return float2(-1.f, 0.f); }
	static float2 right()	{ return float2(1.f , 0.f); }

};

inline std::ostream& operator<< (std::ostream& o,const float2& f)
{
	o << "(" << f.x << ", " << f.y << ")";
	return o;
}

static float dot(float2 a, float2 b) { return a.x * b.x + a.y * b.y; }
static float lengthsq(float2 a) { return a.x * a.x + a.y * a.y; }
inline float length(float2 a) { return std::sqrt(lengthsq(a)); }

inline float2 normalize(const float2& a)
{
	float len = length(a);
	if (len < kEps) return float2::zero();
	return float2(a.x / len, a.y / len);
}

inline float absf(f32 a) { return a < 0.f ? -a : a; }

inline float2 reflect(const float2& v, const float2& n) 
{
	float d = dot(v, n);
	return v - (n * (2.0f * d));
}