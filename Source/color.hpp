#pragma once

struct Color
{
	f32 r{};
	f32 g{};
	f32 b{};
	f32 a{};
	//
	Color() : r(1.f), g(1.f), b(1.f), a(1.f) {};
	Color(f32 red, f32 green, f32 blue) : r(red), g(green), b(blue) { a = 1.f; };
	Color(f32 red, f32 green, f32 blue,f32 alpha) : r(red), g(green), b(blue),a(alpha) {};
	Color(u32 c)
	{
		a = ((c >> 24) & 0xFF) / 255.0f;
		r = ((c >> 16) & 0xFF) / 255.0f;
		g = ((c >> 8) & 0xFF) / 255.0f;
		b = ((c >> 0) & 0xFF) / 255.0f;
	}

	Color(float c[4]) : r(c[0]), g(c[1]), b(c[2]), a(c[3]) {};

	u32 hex()
	{
		u32 ua = static_cast<u32>(a * 255.0f) << 24;
		u32 ur = static_cast<u32>(r * 255.0f) << 16;
		u32 ug = static_cast<u32>(g * 255.0f) << 8;
		u32 ub = static_cast<u32>(b * 255.0f);

		return ua | ur | ug | ub;
	}
};

Color operator+ (const Color& lhs, const Color& rhs);

std::ostream& operator<< (std::ostream& o, const Color& c);