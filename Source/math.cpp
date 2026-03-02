#include "math.hpp"

//operator overload
float2 float2::operator+ (const float2 rhs) const
{
	return float2(x + rhs.x, y + rhs.y);
}

float2 float2::operator-(const float2& rhs) const
{
	return float2(x - rhs.x, y - rhs.y);
}
float2 float2::operator-() const
{
	return float2(-x, -y);
}

float2& float2::operator+=(const float2& rhs)
{
	x += rhs.x;
	y += rhs.y;
	return *this;
}

float2& float2::operator-=(const float2& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	return *this;
}

float2 float2::operator* (const float rhs) const
{
	return float2(x * rhs, y * rhs);
}


std::ostream& operator<< (std::ostream& o,const float2& f)
{
	o << "(" << f.x << ", " << f.y << ")";
	return o;
}

f32 absf(f32 a) { return a < 0.f ? -a : a; }
f32 dot(float2 a, float2 b) { return a.x * b.x + a.y * b.y; }
f32 lengthsq(float2 a) { return a.x * a.x + a.y * a.y; }
f32 length(float2 a) { return std::sqrt(lengthsq(a)); }

float2 normalize(const float2& a)
{
	float len = length(a);
	if (len < kEps) return float2::zero();
	return float2(a.x / len, a.y / len);
}

float2 reflect(const float2& v, const float2& n) 
{
	float d = dot(v, n);
	return v - (n * (2.0f * d));
}

float2 lerp(const float2& a, const float2& b, float t)
{
	return a * (1.f - t) + b * t;
}

namespace Random
{
	float RandFloat(float min, float max)
	{
		float r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
		return min + r * (max - min);
	}

	int RandInt(int min, int max)
	{
		return min + rand() % (max - min + 1);
	}
}

