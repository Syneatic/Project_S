#pragma once
#include "AETypes.h"

struct Color
{
	f32 r{};
	f32 g{};
	f32 b{};
	f32 a{};

	Color() = default;
	Color(f32 red, f32 green, f32 blue) : r(red), g(green), b(blue) { a = 1.f; };
	Color(f32 red, f32 green, f32 blue,f32 alpha) : r(red), g(green), b(blue),a(alpha) {};
};