#pragma once
#include <string>
#include "ImGUI/imgui.h"

#include "component.hpp"

//abstract
struct Collider : Component
{
    //tag
    //isTrigger
};

struct CircleCollider : Collider
{
    f32 radius{ 1.f };

    void DrawInInspector() override
    {
        ImGui::TextUnformatted("Size");
        ImGui::DragFloat("##circlecollider_radius", &radius, 0.1f);
    }

    void Serialize(Json::Value& outComp) const override
    {
        outComp["radius"] = radius;
    }

    void Deserialize(const Json::Value& compObj) override
    {
        if (compObj.isMember("radius") && compObj["radius"].isNumeric())
            radius = compObj["radius"].asFloat();
    }

    const std::string name() const override { return "CircleCollider"; }

    CircleCollider() {};
};

struct BoxCollider : Collider
{
    float2 size{ 1.f,1.f };

    void DrawInInspector() override
    {
        ImGui::TextUnformatted("Size");
        ImGui::DragFloat2("##boxcollider_size", &size.x, 0.1f);
    }

    void Serialize(Json::Value& outComp) const override
    {
        outComp["size"] = WriteFloat2(size);
    }

    void Deserialize(const Json::Value& compObj) override
    {
        if (compObj.isMember("size")) ReadFloat2(compObj["size"], size);
    }

    const std::string name() const override { return "BoxCollider"; }
};