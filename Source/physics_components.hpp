/*
Author: Muhammad Harith Bin Khairudyn
Co-Author: Lim Yan Chun
*/
#pragma once 

#include "physics_types.hpp"

// Bitmask enum representing which physics layer an object belongs to.
enum class Layer : uint32_t
{
    Nothing = 0,
    Player = 1 << 0,
    Environment = 1 << 1,
    Enemy = 1 << 2,
    Projectile = 1 << 3,
    CheckPoint = 1 << 4,
    End = 1 << 5
};
// Converts a Layer enum value to its human-readable string name.
const char* LayerToString(Layer layer);

// Abstract base for all collider types; holds layer, mask, AABB, and OBB data.
class Collider : public Component
{
public:
    bool isTrigger{false};
    u32 layer{ static_cast<u32>(Layer::Nothing) }; //layer that it is on
    u32 collisionMask{ 0xFFFFFFFF }; //layers to collide with
    AABB aabb;
    OBB obb;

    // Registers this collider with the physics system on scene start.
    void OnStart() override;
    // Draws the shared layer, collision mask, and trigger controls in the inspector.
    void DrawBaseInspector();

    Collider(GameObject& go) : Component(go) {};
};

// Axis-aligned box collider with a configurable world-space size.
class BoxCollider : public Collider
{
public:
    float2 size{ 1.f,1.f };
    // Draws size and base collider fields in the inspector.
	void DrawInInspector() override;
    // Serializes trigger, size, layer, and mask to JSON.
	void Serialize(Json::Value& outComp) const override;
    // Deserializes trigger, size, layer, and mask from JSON.
	void Deserialize(const Json::Value& compObj) override;

    const std::string name() const override { return "BoxCollider"; }

    BoxCollider(GameObject& go) : Collider(go) {};
    // Copies all BoxCollider fields from the given source component.
    void CopyFrom(Component* src) override;
    // Returns a deep copy of this BoxCollider attached to the given GameObject.
    std::unique_ptr<Component> Clone(GameObject& go) override;
};

// Adds physics-driven motion to a GameObject via velocity, forces, and gravity.
class RigidBody : public Component
{ 
public:
	bool isStatic{ false };
    bool isKinematic{ false };
	bool useGravity{ false };

	float2 velocity{ 0.0f,0.0f };
    float2 accumulatedForce{};

    // Draws static, kinematic, and gravity toggles in the inspector.
	void DrawInInspector()override;
    // Serializes static, kinematic, and gravity flags to JSON.
	void Serialize(Json::Value& outComp) const override;
    // Deserializes static, kinematic, and gravity flags from JSON.
    void Deserialize(const Json::Value& compObj) override;
    // Registers this RigidBody with the physics system on scene start.
    void OnStart() override;

	const std::string name() const override { return "RigidBody"; }

    RigidBody(GameObject& go) : Component(go) {};
    // Copies gravity, static, and kinematic flags from the given source component.
    void CopyFrom(Component* src) override;
    // Returns a deep copy of this RigidBody attached to the given GameObject.
    std::unique_ptr<Component> Clone(GameObject& go) override;

    //rigidbody interfaces
public:
    // Accumulates an instantaneous force to be applied during the next physics step.
	void AddForce(const float2& force) { accumulatedForce += force; }

};