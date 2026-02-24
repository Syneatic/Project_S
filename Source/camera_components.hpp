#pragma once
#include "AEEngine.h"

#include "camera.hpp"
#include "base_components.hpp"

struct MainCamera : Behaviour
{
	Transform* transform;

	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

	const std::string name() const override { return "Camera"; }
};