#pragma once

#include "base_components.hpp"
#include "math.hpp"
#include "gameobject.hpp"
#include "physics_components.hpp"
#include "emitter_components.hpp"

//abstract
struct Controller : Behaviour
{

};

struct PlayerController : Controller
{
    f32 maxSpeed = 10.f;
    f32 jumpHeight = 500.f;
    f32 time = 1.f;

    f32 dt{};

    Transform* trans = nullptr;
    RigidBody* rb = nullptr;
    GameObject* rockObject = nullptr;

    void DrawInInspector() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;
    void OnStart() override;

    void OnUpdate() override;
    void OnDestroy() override;

    const std::string name() const override { return "PlayerController"; }
};

struct RockController : Controller
{
    enum class RockState
    {
        Idle,
        Thrown,
        Impact
    };
    RockState state = RockState::Idle;

    f32 throwSpeed = 250.f;
    f32 throwAngle = 0.f; //degrees

    float2 dir{0.f, 0.f};

    Transform* trans = nullptr;
    RigidBody* rb = nullptr;

    //ParticleEmitter* emitter = nullptr;

    void DrawInInspector() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;

    //Scene Management Function
    void OnStart() override;
    void OnUpdate() override;
    void OnDestroy() override;

    void Throw(const float2& startPos);
    void OnImpact();
    void ResetRock();

    const std::string name() const override { return "RockController"; }
};