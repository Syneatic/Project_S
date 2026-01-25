#pragma once
#include <string>
#include "ImGUI/imgui.h"

#include "json.h"
#include "json_parser_helper.hpp"

struct GameObject;
struct Collider;
struct Renderer;

struct Component
{
protected:
    GameObject* _owner = nullptr;
    Component() = default;
    explicit Component(GameObject* owner) : _owner(owner) {};

public:
    void SetOwner(GameObject* owner) { _owner = owner; }
    GameObject& gameObject() { return *_owner; }
    const GameObject& gameObject() const { return *_owner; }
    friend class GameObject; //allow GameObject class to access private and protected

	virtual void DrawInInspector() {};
    virtual void Serialize(Json::Value& outComp) const{};
    virtual void Deserialize(const Json::Value& compObj) {};

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
	f32 rotation{};

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
	}

    void Serialize(Json::Value& outComp) const override
    {
        outComp["position"] = WriteFloat2(position);
        outComp["scale"] = WriteFloat2(scale);
        outComp["rotation"] = rotation;
    }

    void Deserialize(const Json::Value& compObj) override
    {
        if (compObj.isMember("position")) ReadFloat2(compObj["position"], position);
        if (compObj.isMember("scale"))    ReadFloat2(compObj["scale"], scale);
        if (compObj.isMember("rotation") && compObj["rotation"].isNumeric())
            rotation = compObj["rotation"].asFloat();
    }

	const std::string name() const override { return "Transform"; }

	Transform() {}

	Transform(float2 pos,float2 scl,f32 rot) : position(pos), scale(scl), rotation(rot)
	{
	}
};
