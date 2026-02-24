#pragma once
#include "AEEngine.h"

#include "camera.hpp"
#include "base_components.hpp"


struct Camera : Component
{
    void DrawInInspector() override;
    void Serialize(Json::Value& outComp) const override;
    void Deserialize(const Json::Value& compObj) override;
    virtual void SetPos();
};

struct MainCamera : Camera
{
    void SetPos() override;

    //no override since sprite is quite normal

    const std::string name() const override { return "Camera"; }
};
