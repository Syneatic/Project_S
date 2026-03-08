#pragma once

#include "math.hpp"
#include "eventhandler.hpp"

class GameObject;
class RigidBody;
class NoiseSource;

//abstract
class Controller : public Component
{
public:
    Controller(GameObject& go) : Component(go) {};
};

class PlayerController : public Controller
{
private:
    bool _isGrounded = false;

public:
    f32 maxSpeed = 150.f;
    f32 jumpHeight = 60.f;
    f32 time = 0.7f;

    float2 spawnPoint {0.f, 0.f};

    RigidBody* rb = nullptr;
    GameObject* rockObject = nullptr;

    void DrawInInspector() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;
    void OnStart() override;

    void OnUpdate() override;
    void OnDestroy() override;

    void HandleCollision(const OnCollisionEvent& e);
    void HandleTrigger(const OnTriggerEvent& e);
    void SaveSpawn(const float2& pos);
    void Respawn();

    const std::string name() const override { return "PlayerController"; }

    PlayerController(GameObject& go) : Controller(go) {};
    void CopyFrom(Component* src) override;
    std::unique_ptr<Component> Clone(GameObject& go) override;
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
    void CopyFrom(Component* src) override;
    std::unique_ptr<Component> Clone(GameObject& go) override;
};

enum class EnemyType
{
    Static,
    Drop,
    Patrol,
    Flying
};

class EnemyController : public Controller
{
public:
    //Global Variable
    f32 groundEmitTimer = 0.f;
    f32 groundEmitInterval = 1.5f;
    GameObject* playerObject = nullptr;

    //Drop Variable
    f32 detectDistance = 50.f;
    bool hasDropped = false;

    //Patrol Variable
    f32 moveSpeed = 100.f;
    f32 patrolRange = 200.f;
    float2 startPos{};
    int patrolDir = 1; // 1 = right, -1 = left

    RigidBody* rb = nullptr;
    NoiseSource* ns = nullptr;

    EnemyType type = EnemyType::Static;

    void DrawInInspector() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;

    //Scene Management Function
    void OnStart() override;
    void OnUpdate() override;
    void OnDestroy() override;

    //Helper Function
    void UpdateDrop();
    void UpdatePatrol();

    const std::string name() const override { return "EnemyController"; }
    EnemyController(GameObject& go) : Controller(go) {};
    void CopyFrom(Component* src) override;
    std::unique_ptr<Component> Clone(GameObject& go) override;
};