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

    const std::string name() const override { return "BoxCollider"; }
};