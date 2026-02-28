#pragma once

#include "math.hpp"

//purely a data container now
struct Transform
{
	float2 position{};
	float2 scale{1.f,1.f};
	f32 rotation{};

    //void DrawInInspector() override;
    //void Serialize(Json::Value& outComp) const override;
    //void Deserialize(const Json::Value& compObj) override;

	//const std::string name() const override { return "Transform"; }

    Transform(float2 pos = float2(), float2 scl = float2(), f32 rot = 0) : position(pos), scale(scl), rotation(rot) {}
};
