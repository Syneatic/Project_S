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