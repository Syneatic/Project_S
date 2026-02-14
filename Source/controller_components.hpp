#pragma once

#include "json.h"
#include "component.hpp"

//abstract
struct Controller : Behaviour
{
    //max speed
    //velocity
    //jump height
    //collidertype
};

struct PlayerController : Controller
{
    f32 maxSpeed = 10.f;
    f32 jumpHeight = 500.f;

    //should not cache like this
    f32 dt = (f32)AEFrameRateControllerGetFrameTime();

    void DrawInInspector() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;
    void OnStart() override;

    void OnUpdate() override;
    void OnDestroy() override;

    const std::string name() const override { return "PlayerController"; }
};