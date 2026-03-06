#pragma once 

#include "physics_types.hpp"

enum class Layer : uint32_t
{
    Nothing = 0,
    Player = 1 << 0,
    Environment = 1 << 1,
    Enemy = 1 << 2,
    Projectile = 1 << 3, //not for raycast for other object projectiles
    CheckPoint = 1 << 4 
};

const char* LayerToString(Layer layer);

//abstract
class Collider : public Component
{
public:
    bool isTrigger{false};
    u32 layer{ static_cast<u32>(Layer::Nothing) }; //layer that it is on
    u32 collisionMask{ 0xFFFFFFFF }; //layers to collide with
    AABB aabb;
    OBB obb;

    //events
    void OnStart() override;

    void DrawBaseInspector();

    Collider(GameObject& go) : Component(go) {};
};

class BoxCollider : public Collider
{
public:
    float2 size{ 1.f,1.f };

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

    const std::string name() const override { return "BoxCollider"; }

    BoxCollider(GameObject& go) : Collider(go) {};
    void CopyFrom(Component* src) override;
    std::unique_ptr<Component> Clone(GameObject& go) override;
};

class RigidBody : public Component
{ 
public:
	bool useGravity{ false };
    bool isKinematic{ false };
    bool detectCollisions{ false };

	bool isStatic{ false };
	bool isGrounded{ false };
	float gravity{ 9.8f };
	float terminalVelocity{ 12.0f };
	float2 velocity{ 0.0f,0.0f };
    float2 accumulatedForce{};

    bool HitEnvironment{ false };
    bool HitEnemy{ false };
    bool HitCheckPoint{ false };
    bool HitProjectile{ false };

	void DrawInInspector()override;
	void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;


    void OnStart() override;
    void Clear_Forces();

	const std::string name() const override { return "RigidBody"; }

    RigidBody(GameObject& go) : Component(go) {};
    void CopyFrom(Component* src) override;
    std::unique_ptr<Component> Clone(GameObject& go) override;
};