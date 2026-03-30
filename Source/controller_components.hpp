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
    float2 spawnPoint{ 0.f, 0.f };
    float2 initialSpawnPoint{ -889.833f , -113.000f };

public:
    f32 maxSpeed = 150.f;
    f32 jumpHeight = 60.f;
    f32 time = 0.7f;

    f32 coyoteTimer = 0.f;
    f32 coyoteMax = 0.15f;

    float2 lastEchoPos{};
    bool wasMoving{ false };
    f32 minEchoDistance{ 100.f };
    f32 echoDistanceThreshold{ 120.f };
    f32 distanceAccumulated{ 0.0f };

    bool isPinging = false;
    float pingActiveTimer{};
    float pingActiveDuration{};
    float2 lastPingPosition{};

    float2 moveStartPos{};

    bool ignoreProjectileCollision{ false };
    f32 ignoreTimer{ 0.f };
    f32 ignoreDuration{ 0.5f };

    f32 currentAlpha = 1.f;
    f32 pingFadeSpeed = 1.5f;
    f32 fadeCooldownTimer = 0.f;
    f32 fadeCooldownDuration = 2.f;
    bool isFadingOut = false;
    bool isFadeCoolingDown = false;

    bool canPing = true;

    RigidBody* rb = nullptr;
    GameObject* rockObject = nullptr;
    NoiseSource* noiseSource = nullptr;

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
    void ResetSpawn();

    float2 GetSpawnPoint() const { return spawnPoint; }
    float2 GetCurrentPosition() const { return _transform.position; }
    void SetSpawnPoint(const float2& pos) { spawnPoint = pos; }

    void TriggerPing(const float2& pos);

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
    f32 detectDistance = 300.f;
    f32 rangeDistance = 100.f;
    bool hasDropped = false;

    //Patrol Variable
    f32 moveSpeed = 100.f;
    f32 patrolRange = 200.f;
    float2 startPos{};
    int patrolDir = 1; // 1 = right, -1 = left

    bool framePing = false;
    bool heardPlayer = false;
    float targetX = 0.f;
    float hearRange = 200.f;
    float hearHeight = 100.f;
    float hearMoveSpeed = 120.f;

    bool investigating = false;
    float2 investigateTarget{};

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
    void CheckPlayerSound();
    void MoveTowardsXPos();

    const std::string name() const override { return "EnemyController"; }
    EnemyController(GameObject& go) : Controller(go) {};
    void CopyFrom(Component* src) override;
    std::unique_ptr<Component> Clone(GameObject& go) override;
};