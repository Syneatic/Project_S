#include "color.hpp"


Color operator+ (const Color& lhs, const Color& rhs)
{
	Color c{};
	c.r = lhs.r + rhs.r;
	c.g = lhs.g + rhs.g;
	c.b = lhs.b + rhs.b;
	c.a = lhs.a + rhs.a;
	return c;
}

std::ostream& operator<< (std::ostream& o, const Color& c)
{
	o << "(" << c.r << ", " << c.g << ", " << c.b << ", " << c.a << ")\n";
	return o;
}

Color Lerp(Color a, Color b, f32 t)
{
	return Color
	{
		a.r + (b.r - a.r) * t,
		a.g + (b.g - a.g) * t,
		a.b + (b.b - a.b) * t,
		a.a + (b.a - a.a) * t
	};
}

Color RandColor(const Color a, const Color b)
{
	float t = Random::RandFloat(0.f, 1.f);
	return Lerp(a, b, t);

}