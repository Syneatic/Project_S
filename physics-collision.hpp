#pragma once
#include "collider_components.hpp"
#include "component.hpp"
#include "math.hpp"


namespace {

	//helper

	float2 rotatePoint(const float2& point, float angle)
	{
		float rad = angle * (3.14159265359f / 180.0f);
		float c = cosf(rad);
		float s = sinf(rad);

		return { point.x * c - point.y * s, point.x * s + point.y * c };
	}

	float dot(const float2& a, const float2& b)
	{
		return a.x * b.x + a.y * b.y;
	}

	void getBoxCorner(const Transform& t, const float2& size, float2 out[4])
	{
		float2 half = { size.x * t.scale.x / 2.0f, size.y * t.scale.y / 2.0f };
		float2 local[4] = {
			 {-half.x, -half.y},
			{ half.x, -half.y},
			{ half.x,  half.y},
			{-half.x,  half.y}
		};

		for (int i = 0; i < 4; i++)
		{
			float2 rotated = rotatePoint(local[i], t.rotation);
			out[i] = { rotated.x + t.position.x, rotated.y + t.position.y };
		}
	}

	/*float2 edgenormal(const float2& a, const float2& b)
	{
		float2 edge = { b.x - a.x, b.y - a.y };
		float2 normal = { -edge.y, edge.x };

	}
	*/
}

namespace Physics
{
	bool CircleVSCircle(const CircleCollider& a, const CircleCollider& b, const Transform& ta, const Transform& tb)
	{
		float dx = tb.position.x - ta.position.x;
		float dy = tb.position.y - ta.position.y;

		float distance_sq = dx * dx + dy * dy;

		float rad_sum = (a.radius * ta.scale.x) + (b.radius * tb.scale.x);

		return distance_sq < (rad_sum * rad_sum);
	}

	bool BoxVSBox(const BoxCollider& a, const BoxCollider& b, const Transform& ta, const Transform& tb)
	{
		float left_b1 = ta.position.x - (a.size.x * ta.scale.x) / 2.0f;
		float right_b1 = ta.position.x + (a.size.x * ta.scale.x) / 2.0f;
		float top_b1 = ta.position.y + (a.size.y * ta.scale.y) / 2.0f;
		float bot_b1 = ta.position.y - (a.size.y * ta.scale.y) / 2.0f;

		float left_b2 = tb.position.x - (b.size.x * tb.scale.x) / 2.0f;
		float right_b2 = tb.position.x + (b.size.x * tb.scale.x) / 2.0f;
		float top_b2 = tb.position.y + (b.size.y * tb.scale.y) / 2.0f;
		float bot_b2 = tb.position.y - (b.size.y * tb.scale.y) / 2.0f;

		return (left_b1 < right_b2 && right_b1 > left_b2 && top_b1 > bot_b2 && bot_b1 < top_b2);
	}
}