#pragma once
#include "collider_components.hpp"
#include "component.hpp"
#include "math.hpp"
#include "scene.hpp"           
#include "gameobject.hpp"      
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <iostream>   

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

	float2 edgeNormal(const float2& a, const float2& b)
	{
		float2 edge = { b.x - a.x, b.y - a.y };
		float2 normal = { -edge.y, edge.x };
		float len = sqrtf(normal.x * normal.x + normal.y * normal.y);

		if (len == 0) return { 0,0 };
		normal.x /= len;
		normal.y /= len;

		return normal;
	}
	
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
		/*float left_b1 = ta.position.x - (a.size.x * ta.scale.x) / 2.0f;
		float right_b1 = ta.position.x + (a.size.x * ta.scale.x) / 2.0f;
		float top_b1 = ta.position.y + (a.size.y * ta.scale.y) / 2.0f;
		float bot_b1 = ta.position.y - (a.size.y * ta.scale.y) / 2.0f;

		float left_b2 = tb.position.x - (b.size.x * tb.scale.x) / 2.0f;
		float right_b2 = tb.position.x + (b.size.x * tb.scale.x) / 2.0f;
		float top_b2 = tb.position.y + (b.size.y * tb.scale.y) / 2.0f;
		float bot_b2 = tb.position.y - (b.size.y * tb.scale.y) / 2.0f;
		*/

		float2 aCorners[4], bCorners[4];
		getBoxCorner(ta, a.size, aCorners);
		getBoxCorner(tb, b.size, bCorners);

		float2 axes[4] = {
			edgeNormal(aCorners[0], aCorners[1]),
			edgeNormal(aCorners[1], aCorners[2]),
			edgeNormal(bCorners[0], bCorners[1]),
			edgeNormal(bCorners[1], bCorners[2])
		};

		for (int i = 0; i < 4; i++)
		{
			float minA = FLT_MAX, maxA = -FLT_MAX;
			float minB = FLT_MAX, maxB = -FLT_MAX;

			for (int j = 0; j < 4; j++)
			{
				float projA = dot(aCorners[j], axes[i]);
				float projB = dot(bCorners[j], axes[i]);

				minA = (std::min)(minA, projA);
				maxA = (std::max)(maxA, projA);

				minB = (std::min)(minB, projB);
				maxB = (std::max)(maxB, projB);
			}

			if (maxA < minB || maxB < minA)
			{
				return false;
			}
		}

		return true;
	}

	void CheckAllTypeCollisions(Scene& scene)
	{
		auto& objects = scene.gameObjectList();
		
		for (size_t i = 0; i < objects.size(); i++)
		{
			for (size_t j = i + 1; j < objects.size(); j++)
			{
				GameObject* obj1 = objects[i].get();
				GameObject* obj2 = objects[j].get();

				if (!obj1 || !obj2) continue;

				Transform* t1 = obj1->GetComponent<Transform>();
				Transform* t2 = obj2->GetComponent<Transform>();

				if (!t1 || !t2) continue;

				BoxCollider* b1 = obj1->GetComponent<BoxCollider>();
				BoxCollider* b2 = obj2->GetComponent<BoxCollider>();

				if (b1 && b2)
				{
					if (BoxVSBox(*b1, *b2, *t1, *t2))
					{
						std::cout << obj1->name() << " and " << obj2->name() << " are colliding! (Box Vs Box)\n";
					}
				}

				CircleCollider* c1 = obj1->GetComponent<CircleCollider>();
				CircleCollider* c2 = obj2->GetComponent<CircleCollider>();

				if (c1 && c2)
				{
					if (CircleVSCircle(*c1, *c2, *t1, *t2))
					{
						std::cout << obj1->name() << " and " << obj2->name() << " are colliding! (Circle Vs Circle)\n";
					}
				}
			}
		}
	}
}