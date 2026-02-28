#pragma once

#include "math.hpp"
#include "eventhandler.hpp"

struct GameObject;
struct RigidBody;
struct NoiseSource;

//abstract
class Controller : public Component
{
public:
    Controller(GameObject& go) : Component(go) {};
};

class PlayerController : public Controller
{
public:
    f32 maxSpeed = 10.f;
    f32 jumpHeight = 500.f;
    f32 time = 1.f;

    f32 dt{};

    RigidBody* rb = nullptr;
    GameObject* rockObject = nullptr;

    void DrawInInspector() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;
    void OnStart() override;

    void OnUpdate() override;
    void OnDestroy() override;

    const std::string name() const override { return "PlayerController"; }

    PlayerController(GameObject& go) : Controller(go) {};
};

class RockController : public Controller
{
public:
    f32 throwSpeed = 250.f;
    f32 throwAngle = 0.f; //degrees

    RigidBody* rb = nullptr;
    NoiseSource* ns = nullptr;

    void DrawInInspector() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;

    //Scene Management Function
    void OnStart() override;
    void OnUpdate() override;
    void OnDestroy() override;

    void Throw(const float2& startPos);
    void OnImpact(const OnCollisionEvent& e);
    void ResetRock();

    const std::string name() const override { return "RockController"; }
    RockController(GameObject& go) : Controller(go) {};
};