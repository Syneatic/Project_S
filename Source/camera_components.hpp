#pragma once

#include "camera.hpp"

struct MainCamera : Component
{
	void OnStart() override;
	void OnUpdate() override;
	void OnDestroy() override;

	void DrawInInspector() override;
	void Serialize(Json::Value& outComp) const override;
	void Deserialize(const Json::Value& compObj) override;

	const std::string name() const override { return "Camera"; }

	MainCamera(GameObject& go) : Component(go) {};
};