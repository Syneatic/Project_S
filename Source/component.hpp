#pragma once
#include "math.hpp"
#include "ImGUI/imgui.h"

struct Component
{
	virtual void DrawInInspector() {};

	virtual const std::string name() const = 0;
	virtual ~Component() = default;
};

struct Behaviour : Component
{
    virtual void OnStart()
    {

    }

    virtual void OnUpdate()
    {

    }

    virtual void OnDestroy()
    {

    }

    virtual ~Behaviour() = default;
};

struct Transform : Component
{
	float2 position{};
	float2 scale{1.f,1.f};
	f32 rotation{}; //stay as float?

    //should only be available in debug mode
	void DrawInInspector() override
	{
        ImGui::TextUnformatted("Position");
        ImGui::DragFloat2("##Position", &position.x, 0.05f);
        ImGui::SameLine();
        if (ImGui::Button("Reset##Pos"))
        {
            position.x = 0.0f;
            position.y = 0.0f;
        }

        ImGui::TextUnformatted("Scale");
        ImGui::DragFloat2("##Scale", &scale.x, 0.05f);
        ImGui::SameLine();
        if (ImGui::Button("Reset##Scale"))
        {
            scale.x = 1.0f;
            scale.y = 1.0f;
        }

        ImGui::TextUnformatted("Rotation");
        ImGui::DragFloat("##Rotation", &rotation, 0.1f);
        ImGui::SameLine();
        if (ImGui::Button("Reset##Rot"))
        {
            rotation = 0.0f;
        }

        //ImGui::Spacing();
        //ImGui::Separator();
        //ImGui::Text("Raw Values:");
        //ImGui::BulletText("Position: (%.3f, %.3f)", position.x, position.y);
        //ImGui::BulletText("Scale:    (%.3f, %.3f)", scale.x, scale.y);
        //ImGui::BulletText("Rotation: %.3f", rotation);
	}

	const std::string name() const override { return "Transform"; }

	Transform() {}

	Transform(float2 pos,float2 scl,f32 rot) : position(pos), scale(scl), rotation(rot)
	{
	}
};

struct Renderer : Component
{
    //texture
    //color multiply
    //render method
    //render layer
};

struct Collider : Component
{
    //tag
    //isTrigger
    //OnEnter
    //OnStay
    //OnExity
};

struct CircleCollider : Collider
{
    f32 radius{ 1.f };
};

struct BoxCollider : Collider
{
    f32 width{ 1.f };
    f32 height{ 1.f };
};