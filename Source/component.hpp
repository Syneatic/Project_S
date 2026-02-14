#pragma once

#include "json.h"
#include "math.hpp"

struct GameObject;

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

    void DrawInInspector() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;

	const std::string name() const override { return "Transform"; }

    Transform(float2 pos = float2(), float2 scl = float2(), f32 rot = 0) : position(pos), scale(scl), rotation(rot) {}
};
