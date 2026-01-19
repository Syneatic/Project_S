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
    virtual void OnStart() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnDestroy() = 0;

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
        ImGui::DragFloat2("##transform_position", &position.x, 0.05f);
        ImGui::SameLine();
        if (ImGui::Button("Reset##Pos"))
        {
            position.x = 0.0f;
            position.y = 0.0f;
        }

        ImGui::TextUnformatted("Scale");
        ImGui::DragFloat2("##transform_scale", &scale.x, 0.05f);
        ImGui::SameLine();
        if (ImGui::Button("Reset##Scale"))
        {
            scale.x = 1.0f;
            scale.y = 1.0f;
        }

        ImGui::TextUnformatted("Rotation");
        ImGui::DragFloat("##transform_rotation", &rotation, 0.1f);
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

//abstract
struct Renderer : Component
{
    //texture
    //color multiply
    //render method
    //render layer
};

//abstract
struct Collider : Component
{
    //tag
    //isTrigger
};

//abstract
struct Controller : Component
{
    //max speed
    //velocity
    //drag
    //gravity
    //jump height
    //collidertype
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

struct PlayerController : Controller
{
    f32 maxspeed{ 200.f };
    f32 jumpHeight{ 20.f };
    float2 velocity{ 1.f, 1.f };
    f32 gravity{ 12.f };
    f32 drag{ 10.f };

    void DrawInInspector() override
    {
        f32 total_width = ImGui::GetContentRegionAvail().x;
        f32 spacing = ImGui::GetStyle().ItemSpacing.x;

        ImGui::TextUnformatted("Max Speed");
        ImGui::DragFloat("##max_speed", &maxspeed, 0.1f, 0.f);

        ImGui::TextUnformatted("Jump Height");
        ImGui::DragFloat("##jumpHeight", &jumpHeight, 0.1f, 0.f);

        ImGui::TextUnformatted("Velocity");
        ImGui::PushItemWidth(((total_width - spacing) /2) - 100.f);
        ImGui::DragFloat("##velocityX", &velocity.x, 0.f, -maxspeed, maxspeed);
        ImGui::SameLine();
        ImGui::DragFloat("##velocityY", &velocity.y, 0.f, -100.f, jumpHeight);
        ImGui::SameLine();
        if (ImGui::Button("Reset##Velocity"))
        {
            velocity.x = 0.0f;
            velocity.y = 0.0f;
        }
        ImGui::PopItemWidth();

        ImGui::TextUnformatted("Gravity");
        ImGui::DragFloat("##gravity", &gravity, 0.1f, 0.f);

        ImGui::TextUnformatted("Drag");
        ImGui::DragFloat("##drag", &drag, 0.1f, 0.f);
    }

    const std::string name() const override { return "PlayerController"; }
};