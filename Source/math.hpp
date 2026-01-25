#pragma once
#include "AETypes.h"

namespace
{
	constexpr float kEps = 1e-6f;
}

struct float2
{
	f32 x{};
	f32 y{};

	float2() : x(0.0f), y(0.0f)
	{
	}

	float2(f32 ix,f32 iy) : x(ix), y(iy)
	{
	}

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
};

static float dot(float2 a, float2 b) { return a.x * b.x + a.y * b.y; }
static float lengthsq(float2 a) { return a.x * a.x + a.y * a.y; }
static float length(float2 a) { return std::sqrt(lengthsq(a)); }

inline float2 normalize(float2 a)
{
	float len = length(a);
	if (len < kEps) return float2::zero();
	return float2(a.x / len, a.y / len);
}

inline float absf(f32 a) { return a < 0.f ? -a : a; }
