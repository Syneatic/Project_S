#pragma once
#include <string>

#include "math.hpp"
#include "ImGUI/imgui.h"

struct GameObject;

struct Component
{
private:
    GameObject* _owner = nullptr;

protected:
    Component() = default;
    explicit Component(GameObject* owner) : _owner(owner) {};

public:
    void SetOwner(GameObject* owner) { _owner = owner; }
    GameObject& gameObject() { return *_owner; }
    const GameObject& gameObject() const { return *_owner; }
    friend class GameObject; //allow GameObject class to access private and protected

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
    void Draw()
    {

    }
};

struct SpriteRenderer : Renderer
{
    const std::string name() const override { return "SpriteRenderer"; }

};

struct MeshRenderer : Renderer
{
    const std::string name() const override { return "MeshRenderer"; }

};

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